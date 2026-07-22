# include "types.h"

/*
it is use to prevent the concurrent access of kerenl shared data structures
for example if a cpu core is access an element of the proc array then other cpu's 
core should not access it concurrently. so before acessing any shared data structures like this
cpu set the lock equal to 1 using the acquire function and then other cpu cores can not access
it until lock become 0 using the release function
*/
struct spinlock{
    uint locked; 

    char *name;

    struct cpu *cpu;
};