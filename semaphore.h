struct wnode {
  struct proc *p; // process waiting on the semaphore
  int type; // type of lock
  struct wnode *next; // next node in the waiting queue
};

struct semaphore {
  int count; // semaphore maximum value
  int value; // current value
  int type; // current type of semaphore
  struct spinlock lk; // lock for the semaphore
  struct wnode *whead; // head of the waiting queue  
  struct proc *head; // head of acquired queue
};
