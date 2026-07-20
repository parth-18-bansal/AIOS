enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE};

// per process state
struct proc{
     char name[16]; // process name
     int pid; // process id

     enum procstate state; // Process State
};