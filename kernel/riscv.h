#include "types.h"

// which hart (core) is this?
static inline uint64
r_mhartid()
{
  uint64 x;
  asm volatile("csrr %0, mhartid" : "=r"(x));
  return x;
}

// read and write tp, the thread pointer, which xv6 uses to hold
// this core's hartid (core number), the index into cpus[].
static inline uint64
r_tp()
{
  uint64 x;
  asm volatile("mv %0, tp" : "=r"(x));
  return x;
}

static inline void
w_tp(uint64 x)
{
  asm volatile("mv tp, %0" : : "r"(x));
}

/*
########################################################
                MACHINE MODE REGISTER
########################################################
*/

// Machine Status Register, mstatus

#define MSTATUS_MPP_MASK (3L << 11) // previous mode.
#define MSTATUS_MPP_M    (3L << 11)
#define MSTATUS_MPP_S    (1L << 11)
#define MSTATUS_MPP_U    (0L << 11)

static inline uint64
r_mstatus()
{
  uint64 x;
  asm volatile("csrr %0, mstatus" : "=r"(x));
  return x;
}

static inline void
w_mstatus(uint64 x)
{
  asm volatile("csrw mstatus, %0" : : "r"(x));
}

// machine exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void
w_mepc(uint64 x)
{
  asm volatile("csrw mepc, %0" : : "r"(x));
}

/*
########################################################
                      INTERRUPTS
########################################################
*/

// Supervisor Interrupt Enable
#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer

static inline uint64
r_sie()
{
  uint64 x;
  asm volatile("csrr %0, sie" : "=r"(x));
  return x;
}

static inline void
w_sie(uint64 x)
{
  asm volatile("csrw sie, %0" : : "r"(x));
}



#define SSTATUS_SIE (1L << 1) //supervisor interrupt enable(global interrupt enable/disable)


static inline uint64 r_sstatus(){
    uint64 x;
    asm volatile("csrr %0, sstatus" : "=r"(x));
    return x;
}

static inline void w_sstatus(uint64 x){
    asm volatile("csrw sstatus, %0" : : "r"(x));
}

static inline void s_sstatus(uint64 x){
    __asm__ __volatile__("csrs sstatus, %0" ::"rK"(x) : "memory");
}

static inline void c_sstatus(uint64 x){
    __asm__ __volatile__("csrc sstatus, %0" ::"rK"(x) : "memory");
}



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
    c_sstatus(SSTATUS_SIE);
}

// stvec stores the address of the trampoline code
// Supervisor Trap-Vector Base Address
// low two bits are mode.
static inline void w_stvec(uint64 x)
{
  asm volatile("csrw stvec, %0" : : "r"(x));
}

static inline uint64 r_stvec()
{
  uint64 x;
  asm volatile("csrr %0, stvec" : "=r"(x));
  return x;
}

// when trap occurs we store the current's pc in sepc
// supervisor exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void w_sepc(uint64 x)
{
  asm volatile("csrw sepc, %0" : : "r"(x));
}

static inline uint64 r_sepc()
{
  uint64 x;
  asm volatile("csrr %0, sepc" : "=r"(x));
  return x;
}


// here we store the cause of the trap in number form, means a number will represent the cause
static inline uint64 r_scause(){
    uint64 x;
    asm volatile("csrr %0, scause" : "=r"(x));
    return x;
}

// Machine Exception Delegation
static inline uint64
r_medeleg()
{
  uint64 x;
  asm volatile("csrr %0, medeleg" : "=r"(x));
  return x;
}

static inline void
w_medeleg(uint64 x)
{
  asm volatile("csrw medeleg, %0" : : "r"(x));
}

// Machine Interrupt Delegation
static inline uint64
r_mideleg()
{
  uint64 x;
  asm volatile("csrr %0, mideleg" : "=r"(x));
  return x;
}

static inline void
w_mideleg(uint64 x)
{
  asm volatile("csrw mideleg, %0" : : "r"(x));
}


/*
########################################################
                      MEMORY
########################################################
*/


#define MENVCFG_ADUE (1L << 61)

static inline uint64 r_menvcfg()
{
  uint64 x;
  asm volatile("csrr %0, 0x30a" : "=r"(x));
  return x;
}

static inline void w_menvcfg(uint64 x)
{
  // asm volatile("csrw menvcfg, %0" : : "r" (x));
  asm volatile("csrw 0x30a, %0" : : "r"(x));
}


// memory protection
static inline void w_pmpcfg0(uint64 x){
    asm volatile("csrw pmpcfg0, %0" : : "r"(x));
}

static inline void w_pmpaddr0(uint64 x){
    asm volatile("csrw pmpaddr0, %0" : : "r"(x));
}





/*
########################################################
                      SATP REGISTER
########################################################
*/

/*
here we are using sv39 address translation scheme, which is represented by 1000(8) and it is defined
in the top 4 bits
*/
#define SATP_SV39 (8L << 60)
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)pagetable) >> 12))

// supervisor address translation and protection;
// holds the address of the page table.
static inline void w_satp(uint64 x)
{
  asm volatile("csrw satp, %0" : : "r"(x));
}

static inline uint64 r_satp()
{
  uint64 x;
  asm volatile("csrr %0, satp" : "=r"(x));
  return x;
}

/*
cpu has TLB which cache the va and pa mapping, this function is use to flush the tlb after changing
the satp value to new page table.
*/
static inline void sfence_vma(){
    asm volatile("sfence.vam zero, zero" ::: "memory");
}



/*
####################################################################
                            PAGE TABLE
####################################################################
*/

typedef uint64 *pagetable_t;
typedef uint64 pte_t;

/*
PTE = page table entry
now each page table entry is 64 bits long where lowest 10 bits represents different flags
and rest of represent the address.
now zeroth is valid bit where 0 means address is not valid and 1 means address is valid
and 1L means 64 bit long 1.(L = long)
*/
#define PTE_V (1L << 0)  //valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)  // user can access

/*
pte = page table entry and each entry in the page table is of 64 bits
out of 64 bits lower 10 bits are used for flags and rest represent the PPN not PA
PPN = page number and pa is physical address, now 12 bits are used for the page the offset
it means that first address of the each page has 12 zeros in the loweres 12 bits, so while storing 
the PA in the page table we remove those 12 zeros so while converting back it into the PA we have to add
the 12 zeros in the last that is why we did << 12 
*/
#define PTE2PA(pte) (((pte) >> 10) << 12)

#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)


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