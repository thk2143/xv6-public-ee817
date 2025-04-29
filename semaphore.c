#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
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
    for (int i=0; i<count; i++)
        lk->pids[i] = 0;
}

static void
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

static struct proc*
dequeue(struct semaphore *lk)
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
    int i;

    acquire(&lk->lk);
    p = myproc();
    enqueue(lk);
    while (lk->whead->pid != p->pid || lk->size == lk->max) {
        sleep(p, &lk->lk);
    }
    if(!dequeue(lk)) {
        panic("downsema: dequeue failed");
    }
    lk->locked = 1;
    lk->size += 1;
    
    i = 0;
    while (i< lk->max){
        if (lk->pids[i] == 0){
            lk->pids[i] = p->pid;
            break;
        }
        i++;
    }
    if (i == lk->max)
        panic("downsema: no space in pids");
    release(&lk->lk);
    return lk->size;
}

int
upsema (struct semaphore *lk){
    struct proc *p;
    int i;

    acquire(&lk->lk);
    p = myproc();
    
    i = 0;
    while (i < lk->max) {
        if (lk->pids[i] == p->pid) {
            lk->pids[i] = 0;
            break;
        }
        i++;
    }
    if (i == lk->max)
        panic("upsema: no pid in pids");

    lk->size -= 1;
    if (lk->size == 0) {
        lk->locked = 0;
    }
    wakeup(lk->whead);
    release(&lk->lk);

    return lk->size;
}