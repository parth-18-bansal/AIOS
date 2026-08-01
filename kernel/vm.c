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
it is used to remove the mapping between the pa and va.
it first empty the the physical page using kfree and then unmap the mapping
*/
void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free){
    uint64 a;
    pte_t *pte;

    if((va % PGSIZE) != 0){
        panic("uvmunmap: not aligned");
    }

    for(a = va; a < va + npages * PGSIZE; a+=PGSIZE){
        if((pte = walk(pagetable,a,0)) == 0){
            continue;
        }
        if((*pte & PTE_V) == 0){
            continue;
        }
        if(do_free){
            uint64 pa = PTE2PA(*pte);
            kfree((void *)pa);
        }
        *pte = 0;
    }
}

/*
summary:
this function delete the pagetable all the level like there are 3 levels so it 
delete all three level. and if there is any mapping in the leaf pagetable then it give
error.
*/
void freewalk(pagetable_t pagetable){
    for (int i = 0; i<512; i++){
        pte_t pte = pagetable[i];
        if((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0){
            uint64 child = PTE2PA(pte);
            freewalk((pagetable_t)child);
            pagetable[i] = 0;
        }
        else if(pte & PTE_V){
            panic("freewalk: leaf");
        }
    }
    kfree((void *)pagetable);
}

/*
summary: this function is used to delete all the mappings of the va and pa also then 
delete the page table at the last
*/
void uvmfree(pagetable_t pagetable, uint64 sz){
    if(sz > 0){
        uvmunmap(pagetable, 0, PGGROUNDUP(sz) / PGSIZE, 1);
    }
    freewalk(pagetable);
}