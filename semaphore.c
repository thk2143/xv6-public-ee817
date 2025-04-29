#include "semaphore.h"

void
initsema (struct semaphore *lk, int count){
    initlock(&lk->lk, "semaphore");
    lk->max = count;
    lk->size = 0;
    lk->type = 0;
    lk->whead = 0;
    lk->head = 0;    
}

int
check (struct semaphore *lk){
    // check current process is at the head of the waiting queue
    if (lk->whead->p->pid != p->pid) 
      return 1;
    // current lock: exclusive
    if (lk->type) 
      return lk->size != 0;
    // current lock: shared
    else {
      // check shared lock is full
      if (lk->size == lk->max)
        return 1;
      // head of wainting is exclusive lock(write) and shared lock is not empty
      if (lk->whead->p->type && lk->size)    
        return 1;
      return 0;
    }
}

int
downsema (struct semaphore *lk){
    struct proc *p, *temp;
    struct wnode *wnode;
    acquire(&lk->lk);
    p = myproc();
    while (check(lk)){ // check current lock is shared lock and exclusive lock is at the head of the waing queue
      sleep(p, &lk->lk);
    }
    wnode = lk->whead;
    lk->size += 1;
    lk->type = wnode ->type;
    
    temp = lk->head;
    lk->head = p;
    p->next = lk->head;
    wakeup(lk->whead->p)
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
    wakeup(lk->whead->p);
    release(&lk->lk);

    return lk->size;
}