#include "types.h"
#include "param.h"
#include "spinlock.h"

/*
trapframe is the page where we store user cpu register values
before cpu start to execute some other process, so that cpu can 
start from the point where it left
*/
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};


/*
it is similar to the trapframe, but trapframe is use let say if cpu is executing the instructions
that are present in the user space and then cpu has to pause the execution of that process
because may be it start execute the another process or it just start executing kernel due to exception
then in that case we save the user's cpu register value but context is use when let say cpu is
executing the kernel code then it pause the execution because it start to execute the another process
then in that case it save some register's value that are used during the kernel execution.
*/
struct context {
  uint64 ra;
  uint64 sp;
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct cpu{
  struct proc *proc; // it value can be null or tell which process cpu is currently running

  /*
  there are two type of context one is process context and other is scheduler context
  there is scheduler function in the kernel code. 
  so let say
  a) proccess A is running, now timer interrupt occur, so cpu first save the user register values into trampframe
  b) now it change CPL(current privilege level to kernel), and execute the kernel code, now afer
  executing the kernel code it will save the registers of the process's kernel code into the 
  proc --> context. Now load the scheduler's register value into the cpu register
  and scheduler will run which let say schedule process B, now after
  scheduler running it save the scheduler's register value into the cpu ---> context and 
  c) run the process's B kernel code to come out from the kernel mode to the user mode and save the
  process B's kernel code register into the proc ---> context and restore the process B 
  user register value to the user cpu register

  Note: also process's kernel code means kernel code is one but due to that process, a particular
  portion of the kernel code ran
  */
  struct context context;

  int noff; // for get the depth of nested pushoff() 
  int intena; //stores does the interrupd enabled earlier
};

extern struct cpu cpu[NCPU];



// this is for the state of the process
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE};

// per process state
struct proc{
     struct spinlock lock;
     char name[16]; // process name
     int pid; // process id

     enum procstate state; // Process State

     struct trapframe *trapframe;
     struct context context;

     /*
     this only include user space memory of the process which covers
     codes, data, heap, stack etc, it does not includes
     process kernel stack, page table, fd table, context, trapframe etc
     */
     uint64 sz;   // size of the process memory

     /*
     while running the process if cpu has to jump to the kernel code
     due to some trap mainly like syscall then to execute the kernel
     code for that process it need the stack and that stack is called the kstack
     */
    uint64 kstack;  // virtual address of the kernel stack
};