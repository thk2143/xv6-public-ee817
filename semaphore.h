struct semaphore {
  uint locked;
  struct spinlock lk;
  int max;
  int size;
  struct proc *whead;

  char *name;
  struct proc *head;
};
