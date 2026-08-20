// kalloc
void* kalloc(void);

// swtch.S
void swtch(struct context *, struct context *);

// proc.c
struct proc*     myproc();

// syscall.c
void             syscall(void);