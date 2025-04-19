// Sleeping locks

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"

void
enqueue(struct sleeplock *lk)
{
  struct proc *node = myproc();
  node->next = 0;

  if (lk->head == 0) {
    lk->head = node;
    lk->tail = node;
  } else {
    lk->tail->next = node;
    lk->tail = node;
  }
}

struct proc*
dequeue(struct sleeplock *lk)
{
  if (lk->head == 0) {
    return 0;
  }

  struct proc *node = lk->head;
  if (lk->head == lk->tail) {
    lk->head = 0;
    lk->tail = 0;
  } else {
    lk->head = node->next;
  }

  return node;
}

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->head = 0;
  lk->tail = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  enqueue(lk);
  while (lk->locked || lk->head->pid != myproc()->pid) {
    sleep(lk, &lk->lk);
  }
  if(!dequeue(lk)) {
    panic("dequeue failed");
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



