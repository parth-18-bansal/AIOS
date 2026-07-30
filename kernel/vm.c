#include "riscv.h"
#include "types.h"
#include "defs.h"

/*
summary
1) pagetable: stores the address of the root page table
2) va = virtual address
3) alloc = if any level's page table does not exist then it tell whether to create a new page table or not

here 1) first we are checking that va is smaller then maxva or not
2) traverse the loop and extracting the 9bit part of the va and then getting the enty that is pa 
and this is how we are traversing to the leaf page table
*/
pte_t * walk(pagetable_t pagetable, uint64 va, int alloc){
    // checking whether va is smaller than maxva or not
    if(va>=MAXVA){
        panic("walk");
    }

    for(int level = 2; level>0; level--){
        pte_t *pte = &pagetable[PX(level,va)];
        if(*pte & PTE_V){
            pagetable = (pagetable_t)PTE2PA(*pte);
        }
        else{
            if(!alloc || (pagetable = (pde_t *)kalloc()) == 0){
                return 0;
            }

            memset(pagetable,0,PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }

    return &pagetable[PX(0,va)];
}




/*
summary:
here pagetable is the address of the root page table, va = virtual address,
pa = physical address, size = it is how many pages we want to map it is always divisible of 
4096 bytes, perm = permission flag

it maps the virtual page with the physical page: means it physical address of the page in the 
leaf page table.
*/
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm){
    uint64 a, last;
    pte_t *pte;

    if ((va % PGSIZE) != 0){
        panic("mappages: va not aligned");
    }

    if((size % PGSIZE) != 0){
        panic("mappages: size not aligned");
    }

    if(size == 0){
        panic("mappages: size");
    }

    a = va;
    last = va + size - PGSIZE;

    for(;;){
        if((pte = walk(pagetable, a, 1)) == 0){
            return -1;
        }

        if(*pte & PTE_V){
            panic("mappages: remap");
        }

        *pte = PA2PTE(pa) | perm | PTE_V;

        if(a == last){
            break;
        }

        a += PGSIZE;
        pa += PGSIZE;
    }

    return 0;
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

/*
summary:

*/