#include "param.h"

struct semaphore {
  uint locked;
  struct spinlock lk;
  int max;
  int size;
  struct proc *whead;

  char *name;
  int pids[NPROC];
};
