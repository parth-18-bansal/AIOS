struct inode;
struct proc;
struct spinlock;

// console.c
void                consoleintr(int);
void                consoleputc(int);

// kalloc
void*               kalloc(void);

// proc.c
struct proc*        myproc();
void                procdump();
int                 either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int                 either_copyin(void *dst, int user_src, uint64 src, uint64 len);

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