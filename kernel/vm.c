#include "riscv.h"
#include "types.h"
#include "defs.h"
#include "memlayout.h"


/*

Physical Memory

+---------------------------+ 0x80000000
| Kernel code (.text)       |
+---------------------------+
| Kernel data (.data/.bss)  |
+---------------------------+
| End of kernel image       |  <-- end (defined by the linker)
+---------------------------+
| Free physical pages       |  <-- kalloc() allocates from here
|                           |
|                           |
|                           |
+---------------------------+
| Remaining RAM             |
+---------------------------+
PHYSTOP


functions with uvm belongs to the user page table and function with kvm belongs to the
kernel page table. And walk, mappages, walkaddr, copyout, copyin, freewalk are common functions.
*/

/*
functions start with kvm maniputates the kernel page table
*/


pagetable_t kernel_pagetable;


/*
########################################################
            KERNEL PAGE TABLE FUNCTIONS
########################################################
*/

/*
it creates kernel page table and page table is directly mapped means va and pa are same
summary:
1) first it creates kernel page table using kalloc and memset
2) then it maps the uart registers with the virutal addresses, uart is use for the input
output
3) then it maps the virtio with the virtual address
4) then it maps the plic hardware 
5) then kernel code and kernel data then trampoline
7) then kernel stacks of processes
*/
pagetable_t kvmmake(void){

    pagetable_t kpgtbl;
    kpgtbl = (pagetable_t)kalloc();
    memset(kpgtbl, 0, PGSIZE);

    /*
    uart does not have physical page in the ram, pa refer to the uart chip's register
    */
    kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R| PTE_W);
    
    /*
    virtio is the virtual disk
    */
    kvmmap(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

    /*
    plic is interrupt controller hardware, when any device like disk etc interrupt
    then that interrupt is first receive by the plic and plic tells cpu
    about the interupt
    */
    kvmmap(kpgtbl, PLIC, PLIC, 0x4000000, PTE_R | PTE_W);

    /*
    map kernel text executable and read only
    etext is the end of the kernel code in the memory
    */
    kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_R | PTE_X);

    /*
    map kernel data and the physical ram we will make use of
    it represents the ram after the kernel code, this portion contains the kernel data
    user pages etc
    */
    kvmmap(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP - (uint64)etext, PTE_R | PTE_W);

    kvmmap(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

    proc_mapstacks(kpgtbl);
}



/*
summary: it add a mapping in the kernel page table and only used during booting
*/
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm){
    if(mappages(kpgtbl, va, sz, pa, perm) != 0){
        panic("kvmap");
    }    
}


/*
it creates the kernel pagetable, and there is one kernel page table
and it is shared by all cpu cores.
*/
void kvminit(void){
    kernel_pagetable = kvmmake();
}


/*
summary: earlier in the start function we disable the paging, now we have enable the paging
also set satp register value to the kernel page table. so this function we run after creating
kernel pagetable(kvminit())
*/
void kvminithart(){
    // wait for any previous writes to the page table memory to finish
    sfence_vma();

    w_satp(MAKE_SATP(kernel_pagetable));

    // flush the stale entries from the TLB
    sfence_vma();

}



/*
########################################################
                COMMON FUNCTIONS
########################################################
*/

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
########################################################
           USER PAGE TABLE FUNCTIONS
########################################################
*/

// UVM = user virtual memory, fuctions starting with uvm manipulates the 
// user page table

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
summary: this function is used to delete all the mappings of the va and pa also then 
delete the page table at the last
*/
void uvmfree(pagetable_t pagetable, uint64 sz){
    if(sz > 0){
        uvmunmap(pagetable, 0, PGGROUNDUP(sz) / PGSIZE, 1);
    }
    freewalk(pagetable);
}


/*
summary:
1) it is here oldsz < newsz (must)
2) then it allocate a new page to that process til oldsz < newsz
3) and map that page to that process via pagetable
*/
uint64 uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm){
    char *mem;
    uint64 a;

    if(oldsz > newsz){
        return oldsz;
    }

    PGGROUNDUP(oldsz);

    for(a = oldsz; a < newsz ; a +=PGSIZE){
        mem = kalloc();

        if(mem == 0){
            uvmdealloc(pagetable, a, oldsz);
            return 0;
        }

        memset(mem, 0, PGSIZE);

        if(mappages(pagetable, a, PGSIZE, (uint64)mem, PTE_R | PTE_U | xperm) != 0){
            kfree(mem);
            uvmdealloc(pagetable, a, oldsz);
            return 0;
        }
    }

    return newsz;

}


/*
summary:
1) it is use to deallocate the page to shrink the memory use by a process
2) it take newsz as the input then from that calculate the number of pages to deallocate
3) and then deallocate them using the uvmunmap function.
*/
uint64 uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz){
    if(newsz >= oldsz){
        return oldsz;
    }

    if(PGGROUNDUP(newsz) < PGGROUNDUP(oldsz)){
        int npages = (PGGROUNDUP(oldsz) - PGGROUNDUP(newsz)) / PGSIZE;
        uvmunmap(pagetable, PGGROUNDUP(newsz), npages, 1 );
    }

    return newsz;
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

