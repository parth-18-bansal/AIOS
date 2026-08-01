a) First firmware and bootloader run and they do few things and load the kernel in the memory.
b) After that, execution of the kernel starts, kernel execution starts from the entry.S file.
c) Each cpu core execute entry.S file, by this file each cpu cores has its own stack, which is needed to run the functions.
d) after this start() run, this file gives privileges to the supervisor mode and exits from the machine mode and enter into the supervisor mode, privileges like memory access, interrupt, exception handling.
start() is also run by each cpu core.
e) after this main() run, which calls different functions
f) first main() calls the userinit(). {this is call by only cpu 0}
g) userinit(): it creates the first user process
