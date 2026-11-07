struct inode;
struct proc;
struct spinlock;

// console.c
void                consoleintr(int);

// kalloc
void*               kalloc(void);

// swtch.S
void                swtch(struct context *, struct context *);

// proc.c
struct proc*        myproc();

// syscall.c
void                syscall(void);

// trap.c
void                prepare_return(void);

// proc.c
struct proc*        myproc();

// string.c
void*               memmove(void*, const void*, uint);
void*               memset(void*, int, uint);


// uart.c
void                uartintr(void);