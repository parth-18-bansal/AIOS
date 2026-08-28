struct inode;
struct proc;
struct spinlock;



// kalloc
void* kalloc(void);

// swtch.S
void swtch(struct context *, struct context *);

// proc.c
struct proc*     myproc();

// syscall.c
void             syscall(void);

// trap.c
void             prepare_return(void);

// string.c
void* memmove(void*, const void*, uint);
void* memset(void*, int, uint);