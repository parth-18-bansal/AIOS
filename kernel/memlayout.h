// physical memory layout

/*
we have physical address space and we assign a portion of it to the RAM, Disk and lly to other
i/o devices like uart etc

things work like this when cpu get any load/store instruction then it send this instruction to the 
address checker via data bus then address checker based on the address range redirect that
instruction to the RAM, Disk, UART etc.

so here we are defining like this:

10000000 -- uart0
*/

// uart0 is the starting address of a virtual page whose offset addresses
// are mapped with uart registers
#define UART0 0x10000000L

// VIRTIO0 is the starting address of a  virtual page.
// virtio is the virtual disk
#define VIRTIO0 0x10001000

// PLIC = platform level interrupt controller
#define PLIC 0x0c000000L

// this is ram starting address, here kernel code starts
#define KERNBASE 0x80000000L

// this is RAM last address
#define PHYSTOP (KERNBASE + 128*1024*1024)

/*
SO trampoline is in the last virtual memory page
*/
#define TRAMPOLINE (MAXVA - PGSIZE)


/*
there is a kernel stack for each process and here we are calculating
the va for a kernel stack

here we are multiplying the pgsize because there is empty page after each stack
for guarding one stack from the other 

it is like this:
               HIGH VIRTUAL ADDRESS
                        │
                        ▼
              ┌──────────────────┐
              │   TRAMPOLINE     │
              ├──────────────────┤
              │                  │
              │  guard page      │
              │  (unmapped)      │
              ├──────────────────┤
              │  kernel stack 0  │
              │     1 page       │
              ├──────────────────┤
              │                  │
              │  guard page      │
              │  (unmapped)      │
              ├──────────────────┤
              │  kernel stack 1  │
              │     1 page       │
              ├──────────────────┤
              │                  │
              │  guard page      │
              ├──────────────────┤
              │  kernel stack 2  │
              │     1 page       │
              └──────────────────┘
                 LOW VIRTUAL ADDRESS
                 
*/
#define KSTACK(p) (TRAMPOLINE - ((p)+1) * 2 * PGSIZE)