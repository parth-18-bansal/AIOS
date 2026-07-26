#include "spinlock.h"

void freerange(void *pa_start, void *pa_end);

struct run{
    struct run *next;
};

struct{
    struct spinlock lock;
    struct run *freelist;
}kmem;


/*
summary: this is call at the boot time which set the value of the kmem struct and 
call the freerange which in turn creates the linkedlist of the free memory pages.
*/
void kinit(){
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
}

/*
summary: so it run at the boot time, and here we pass the first after the
kernel(means that in memory kernel code is stored so pa_start is the address just after that kernel)
so here we pass two address one is of the memory which is just after the kernel and other is last address
of the memory. and then we create the linked list of the free memory pages.
*/
void freerange(void *pa_start, void *pa_end){
    char *p;
    p = (char *)PGGROUNDUP((uint64)pa_start);
    for(;p + PGSIZE <= (char *)pa_end; p += PGSIZE){
        kfree(p);
    }
}


/*
summary: it is used to free the memory page and add that page into the freelist
so it first take the address of the page that we want to free then we erase the data inside
it by filling the junk data and then attach that page to the freelist(which is a linkedlist).

*/
void kfree(void *pa){
    struct run *r;

    if(((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP){
        panic("kfree");
    }

    // fill with junk values
    memset(pa, 1, PGSIZE);

    r = (struct run *)pa;
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist=r;
    release(&kmem.lock);
}


/*
summary: return a starting address of a unused memory page
kmem.freelist point to the new memory page starting page so we are equating it with the 
r and then this new page stores the struct run which point to the next new page starting address
so it equate the kem.freelist to the address of the next new page.

Memset it is used to the fill the memory bytes with any value. so we are storing the junk value
5 in the new page.
*/
void * kalloc(){
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if(r){
        kmem.freelist = r->next;
    }
    release(&kmem.lock);

    if(r){
        memset((char *)r, 5, PGSIZE); // storing the junk in the new page
    }

    return (void *)r;
}