#include "riscv.h"

/*
summary
1) 
*/
pte_t * walk(pagetable_t pagetable, uint64 va, int alloc){
    // checking whether va is smaller than maxva or not
    if(va>=MAXVA){
        panic("walk");
    }

    for(int level = 2; level>0; level--){
        pte_t *pte = &pagetable[PX(level,va)];
        if(*pte & PTE_V){

        }
        else{
            
        }
    }
}


/*
summary:
*/
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm){

}


/*
this function only return a new empty page where we store the pagetable.
*/
pagetable_t uvmcreate(){
    pagetable_t pagetable;

    pagetable = (pagetable_t)kalloc();

    if(pagetable == 0){
        return 0;
    }

    memset(pagetable, 0, PGSIZE);
    return pagetable;
}