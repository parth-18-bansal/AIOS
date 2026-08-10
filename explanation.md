A) First firmware and bootloader run and they do few things and load the kernel in the memory.

B) After that, execution of the kernel starts, kernel execution starts from the entry.S file.

C) Each cpu core execute entry.S file, by this file each cpu cores has its own stack, which is needed to run the functions.

D) after this start() run, this file gives privileges to the supervisor mode and exits from the machine mode and enter into the supervisor mode, privileges like memory access, interrupt, exception handling.
start() is also run by each cpu core.

E) after this main() run, which calls different functions

F) first main() calls the userinit(). {this is call by only cpu 0}

G) kvminit(): it creates the kernel page table and map the trampoline, uart, plic, virtual  disk, kernel code and data and kernel stacks.

H) after that in main we run kvminithart(), it turns on the paging 

I) userinit(): it creates the first user process
