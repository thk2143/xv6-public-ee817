#include "types.h"
#include "defs.h"
#include "param.h"
#include "proc.h"
#include "spinlock.h"
#include "semaphore.h"

void
initsema (struct semaphore *lk, int count){
    initlock(&lk->lk, "semaphore");
    if (count > NPROC)
        panic("semaphore: count > NPROC");
    lk->locked = 0;
    lk->max = count;
    lk->size = 0;    
    lk->whead = 0;
    lk->head = 0;    
}

void
enqueue(struct semaphore *lk)
{
  struct proc *node = myproc();
  node->next = 0;

  if (lk->whead == 0) {
    lk->whead = node;
  } else {
    struct proc *temp = lk->whead;
    while (temp->next != 0) {
      temp = temp->next;
    }
    temp->next = node;
  }
}

struct proc*
dequeue(struct sleeplock *lk)
{
  if (lk->whead == 0) {
    return 0;
  }

  struct proc *node = lk->whead;
  lk->whead = node->next;

  return node;
}

int
downsema (struct semaphore *lk){
    struct proc *p;

    acquire(&lk->lk);
    p = myproc();
    enqueue(lk);
    while (lk->locked || lk->head->pid != p->pid || lk->size == lk->max) {
        sleep(p, &lk->lk);
    }
    if(!dequeue(lk)) {
        panic("downsema: dequeue failed");
    }
    lk->locked = 1;
    lk->size += 1;
    p->next = lk->head;
    lk->head = p;
    release(&lk->lk);
    return lk->size;
}

int
upsema (struct semaphore *lk){
    struct proc *node, *prev;

    acquire(&lk->lk);
    if (lk->head == 0) {
        panic("upsema: no process in acquired queue");
    }
    node = lk->head;
    while(node != myproc()) {
        prev = node;
        node = node->next;
        if (node == 0) {
            panic("upsema: process not found in acquired queue");
        }
    }
    prev->next = node->next;
    node->next = 0;
    lk->size--;
    if (!lk->size) {
        lk->locked = 0;
    }
    wakeup(lk->whead);
    release(&lk->lk);

    return lk->size;
}