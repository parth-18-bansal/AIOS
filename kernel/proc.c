#include "param.h"
#include "proc.h"
#include "spinlock.h"

/*
creating the process array that will stores info about each process
here each element of the array represent one process
*/
struct proc proc[NPROC];
struct cpu cpus[NCPU];
struct spinlock pid_lock;

int nextpid=1;

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

int allocpid(){
    int pid;

    acquire(&pid_lock);
    pid = nextpid;
    nextpid = nextpid + 1;
    release(&pid_lock);

    return pid;
}

/*
summary: 
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

        

    }


}

/*
summary: 
*/
void userinit(void){
    struct proc *p;
    
    
}