#include "param.h"
#include "proc.h"
#include "spinlock.h"
#include "memlayout.h"

/*
creating the process array that will stores info about each process
here each element of the array represent one process
*/
struct proc proc[NPROC];
struct cpu cpus[NCPU];
struct spinlock pid_lock;

struct proc *initproc;

int nextpid=1;

extern void forkret(void);


/*
here we are allocting a page for the process's kernel stack and mapping the va
and pa of it in the kernel page table

each kernel stack page is followed by a guard page(unmapped empty page)
*/
void proc_mapstacks(pagetable_t kpgtbl){
    struct proc *p;

    for(p = proc; p<&proc[NPROC]; p++){
        char *pa = kalloc();

        if(pa == 0){
            panic("kalloc");
        }

        uint64 va = KSTACK((int)(p-proc));

        kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
    }
}

/*
here cpuid should be run with interrupt disable, because let say in mycpu function upto id = cpuid();
run and then interrupt occur due to which that process now run on cpu 2, in that case id = 1
but on cpu2 id will remain 1 but process is running in cpu 2.
*/
int cpuid(){
    int id = r_tp();
    return id;
}

struct cpu * mycpu(void){
    int id = cpuid();
    struct cpu *c = &cpus[id];
    return c;
}

// it returns the current proccess which cpu is executing
struct proc * myproc(void){
    pushoff();
    struct cpu *c = mycpu();
    struct proc *p = c->proc;
    pop_off();
    return p;
}


int allocpid(){
    int pid;

    acquire(&pid_lock);
    pid = nextpid;
    nextpid = nextpid + 1;
    release(&pid_lock);

    return pid;
}

/*
summary
1) creating the pointer to the proc struct and setting it value line wise using the for loop
2) in for loop it is finding first process whose state is UNUSED after that it if found then
it the found label which set different fields of the proc struct
3) and then it return the that proc virtual address.
*/

static struct proc * allocproc(void){
    struct proc *p;

    for(p = proc; p < &proc[NPROC]; p++){
        acquire(&p->lock);
        /*
        procstate is a enum, and it's element represent an integer
        UNUSED is the first element so UNUSED = 0; now when we initialise the proc array 
        above then all elements are proc struct with all fields of the the proc struct is equal to 
        zero, so all proc struct's state field is initially equal to UNUSED.
        */
        if(&p->state==UNUSED){
            goto found;
        }
        else{
            release(&p->lock);
        }
        return 0;

        found:
        p->pid = allocpid();
        p->state = USED;

        //Allocate the trapframe page
        /*
        1) kalloc gives the virtual address of the new page
        (here physical address because va and pa are directly mapped)
        so here we are getting a new page and then we are storing it's 
        address in the p->trapframe. and if kalloc return 0 then it means
        there is no memory left, so we set all the fields in the proc's struct
        to zero back
        */
        if ((p->trapframe = (struct trapframe *)kalloc()) == 0){
            freeproc(p);
            release(&p->lock);
            return 0;
        }


        /*
        here we are assiging the page table to the process
        */
        p->pagetable = proc_pagetable(p);
        if(p->pagetable == 0){
            freeproc(p);
            release(&p->lock);
            return 0;
        }

        memset(&p->context, 0, sizeof(p->context));

        /*
        here ra = return address so it define where should the cpu should return after 
        executing the kernel function.
        */
        p->context.ra = (uint64)forkret;

        /*
        sp = stack pointer register it store the top of the stack
        now here stack stores downwared that is why we are adding the PGSIZE in the 
        p->kstack.
        */
        p->context.sp = p->kstack + PGSIZE;

        return p;
    }


}

/*
summary:- it set the value of all the fields in the proc struct to zero. 
*/
static void freeproc(struct proc *p){
    // here we are freeing the trapframe page if it is occupied
    if(p->trapframe){
        kfree((void *)p->trapframe);
    }
    p->trapframe = 0;

    if(p->pagetable){
        proc_freepagetable(p->pagetable, p->sz);
    }

    p->pagetable = 0;
    p->sz = 0;
    p->pid = 0;
    p->parent = 0;
    p->name[0] = 0;
    p->chan = 0;
    p->killed = 0;
    p->xstate = 0;
    p->state=UNUSED;
}

/*
summary
1) first it create the empty page for the pagetable using the uvmcreate function
and storing its address in the pagetable variable.
2) 
*/
pagetable_t proc_pagetable(){
    pagetable_t pagetable;

    // uvmcreate the root page table so here pagetable stores the address 
    // of the root page table. other levels page tables will be created ondemand 
    // by the walk function.
    pagetable = uvmcreate();

    if(pagetable == 0){
        return 0;
    }


    /*
    here we are mapping the va and pa for the trampoline page
    */
    if(mappages(pagetable, TRAMPOLINE, PGSIZE, (uint64)trampoline, PTE_R | PTE_X) < 0){
        uvmfree(pagetable, 0);
        return 0;
    }

    /*
    here we are mapping the va and pa for the trapframe page
    */
    if(mappages(pagetable, TRAPFRAME, PGSIZE, (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
        uvmummap(pagetable, TRAMPOLINE, 1, 0);
        uvmfree(pagetable, 0);
        return 0;
    }

    return pagetable;
}

/*
summary: 
*/
void userinit(void){
    struct proc *p;

    p = allocprod();
    initproc = p;
}


/*
summary:
*/
void forkret(void){

}

/*
summary
*/
void sched(void){
    
}

/*
summary:
*/
void sleep(void *chan, struct spinlock *lk){

}

/*
summary:
*/
void wakeup(void *chan){
    
}