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