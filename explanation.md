a) First firmware and bootloader run and they do few things and load the kernel in the memory.
b) After that, execution of the kernel starts, kernel execution starts from the entry.S file.
c) Each cpu core execute entry.S file, by this file each cpu cores has its own stack, which is needed to run the functions.
