struct inode;
struct proc;
struct spinlock;

// console.c
void                consoleintr(int);

// kalloc
void*               kalloc(void);

// proc.c
struct proc*        myproc();
void                procdump();

// string.c
void*               memmove(void*, const void*, uint);
void*               memset(void*, int, uint);

// syscall.c
void                syscall(void);

// swtch.S
void                swtch(struct context *, struct context *);

// trap.c
void                prepare_return(void);

// uart.c
void                uartintr(void);


// number of element in fixed size array
#define NELEM(x) (sizeof(x) / sizeof(x[0]))