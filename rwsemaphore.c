#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "semaphore.h"
#include "rwsemaphore.h"

static void
enqueue(struct rwsemaphore *lk, int type) {
    struct node node;
    node.p = myproc();
    node.type = type;
    node.next = 0;
    if(lk->head == 0) {
        lk->head = &node;
        lk->tail = &node;
    } else {
        lk->tail->next = &node;
        lk->tail = &node;
    }
}

static struct node*
dequeue(struct rwsemaphore *lk) {
    if (lk->head == 0) {
        panic("rwsemaphore: dequeue failed");
    }
    struct node *node = lk->head;
    if (lk->head == lk->tail) {
        lk->head = 0;
        lk->tail = 0;
    } else {
        lk->head = lk->head->next;
    }

    return node;
}

void
initrwsema(struct rwsemaphore *lk) {
    initlock(&lk->lk, "rwsemaphore");
    lk->locked = 0;
    lk->type = 0;
    lk->max = 10; // arbitrary value < NPROC
    lk->size = 0;
    initsema(&lk->rsema, lk->max);
    initsema(&lk->wsema, 1);
    lk->head = 0;
    lk->tail = 0;
}

void
downreadsema(struct rwsemaphore *lk) {
    struct proc *p;

    acquire(&lk->lk);
    p = myproc();
    enqueue(lk, 0);   
    while ((lk->locked && lk->type) || lk->head->p->pid != p->pid || lk->size == lk->max) {
        sleep(p, &lk->lk);
    }
    
    // dequeue
    struct node *node = dequeue(lk);
    if (node->type || node->p->pid != p->pid) {
        panic("rwsemaphore: dequeue failed");
    }

    lk->locked = 1;
    lk->type = 0;
    lk->size = downsema(&lk->rsema);
    release(&lk->lk);
}

void
upreadsema(struct rwsemaphore *lk) {
    acquire(&lk->lk);
    lk->size = upsema(&lk->rsema);
    if (lk->size == 0) {
        lk->locked = 0;
        lk->type = 0;
    }
    wakeup(lk->head->p);   
    release(&lk->lk);
}

void
downwritesema(struct rwsemaphore *lk) {
    struct proc *p;

    acquire(&lk->lk);
    p = myproc();
    
    // enqueue
    enqueue(lk, 1);
    
    while (lk->locked || lk->head->p->pid != p->pid || lk->size) {
        sleep(p, &lk->lk);
    }
    
    // dequeue
    struct node *node = dequeue(lk);
    if (!node->type || node->p->pid != p->pid) {
        panic("rwsemaphore: dequeue failed");
    }

    lk->locked = 1;
    lk->type = 1;
    lk->size = downsema(&lk->wsema);
    release(&lk->lk);
}

void
upwritesema(struct rwsemaphore *lk) {
    acquire(&lk->lk);
    lk->size = upsema(&lk->wsema);
    if (lk->size) {
        panic("rwsemaphore: upwritesema failed");
    }
    lk->locked = 0;
    lk->type = 0;
    wakeup(lk->head->p);   
    release(&lk->lk);
}
