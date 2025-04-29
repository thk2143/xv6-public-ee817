struct node {
    struct proc *p;
    int type;
    struct node *next;
};

struct rwsemaphore {
    uint locked;
    struct spinlock lk;
    int type;
    int max;
    int size;
    struct semaphore rsema;
    struct semaphore wsema;
    struct node *head;
    struct node *tail;
};