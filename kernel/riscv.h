#include "types.h"


// checking interrupt is enable or not
static inline int intr_get(){
    uint64 x = r_sstatus();
    return (x & SSTATUS_SIE) != 0;
}

// enable the interrupt
static inline void intr_on(){
    s_sstatus(SSTATUS_SIE);
}

// disable the interrupt
static inline void intr_off(){
    c_sstatus(STATUS_SIE);
}


typedef uint64 *pagetable_t;
typedef uint64 pte_t;

/*
PTE = page table entry
now each page table entry is 64 bits long where lowest 10 bits represents different flags
and rest of represent the address.
now zeroth is valid bit where 0 means address is not valid and 1 means address is valid
and 1L means 64 bit long 1.(L = long)
*/
#define PTE_V (1L << 0)


#define PGSHIFT 12 // page offset
#define PGSIZE 4096 // page size in bytes

/*
so here we are extracting the 9bits of the va corresponding the page table level
*/
#define PGMASK 0x1FF // 111111111 9 times 1
#define PXSHIFT(level) (PGSHIFT + (9*(level)))
#define PX(level, va) ((((uint64)(va)) >> PXSHIFT(level)) & PGMASK)

/*
maximum value of the virtual address
here 9,9,9,12 represent how we divide the va address bits
and we subtract the 1 because va is 64 bits long, from which 27 bits are use for the page number
and 12 bits are for the page offset and rest are 0. now from 27 bits in the xv6, first bit top most
is sign bit and it value is always 0 so root level page table has only 256 entries not 512 entries because
top bit is fixed and only has 0 value.
*/
#define MAXVA (1L << (9+9+9+12-1))