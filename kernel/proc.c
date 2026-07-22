#include "param.h"
#include "proc.h"

/*
creating the process array that will stores info about each process
here each element of the array represent one process
*/
struct proc proc[NPROC];
struct cpu cpus[NCPU];


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

/*
summary: 
*/

static struct proc * allocproc(void){
    struct proc *p;

    for(p = proc; p < &proc[NPROC]; p++){

    }


}

/*
summary: 
*/
void userinit(void){
    struct proc *p;
    
    
}