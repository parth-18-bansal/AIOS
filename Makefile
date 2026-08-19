K = kernel
U = user


# objs is list of the whitespace seperated list of the words.
# end here it get expand like this kernel/vm.o
OBJS = \
	$K/entry.o \
	$K/start.o \
	$K/kalloc.o \
	$K/spinlock.o \
	$K/string.o \
	$K/main.o \
	$K/vm.o \
	$K/proc.o \
	$K/swtch.o \
	$K/trampoline.o

# objdump is the linux tool to inspect the object files, we use it for inspect and understanding
# the object file if we do not have source file.

# riscv64-unknown-elf-objdump etc.are riscv version of the objdump tool

# -i flag give information about the architecture and binary format supported by the objdump

# so here we are checking which objdump(version) support elf64-big binary format.
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-elf-'; \
	elif riscv64-none-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-none-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find a riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

QEMU = qemu-system-riscv64
MIN_QEMU_VERSION = 7.2

# cc = c compiler, it is used to compile the c files into .o files
CC = $(TOOLPREFIX)gcc

# ld = linker, it is used to combine object files into one final executable file
# a.o + b.0 + ... = produce final executable file(after combing them, resolving references etc)
LD = $(TOOLPREFIX)ld

# objcopy = it is used to convert or manipulate the object and executable files
OBJCOPY = $(TOOLPREFIX)objcopy

# objdump
OBJDUMP = $(TOOLPREFIX)objdump


# these are the different flags that we will use with the c compiler and linker command
CFLAGS = -Wall -Werror -Wno-unknown-attributes -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -march=rv64gc
CFLAGS += -std=gnu99
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding
CFLAGS += -fno-common -nostdlib
CFLAGS += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS += -fno-builtin-free
CFLAGS += -fno-builtin-memcpy -Wno-main
CFLAGS += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096



#
########################################################
           # linking
########################################################
#

#kernel.ld is the linker script
# kernel.ld tells the linker where different sections of the kernel should be placed in the 
# memory

# here first using the kernel.ld we linked all the object files and store final 
# executable file in the kernel/kernel

# after that we generate the kernel.asm file which contains the source code version
# of the final execuatable file in the assembly version
# we can use it for debugging

# after that we create the kernel.sym file which contains the symbol table
# i.e function_name = memory address mapping, which we can use for the debugging.
$K/kernel: $(OBJS) $K/kernel.ld
		$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $(OBJS)
		$(OBJDUMP) -S $K/kernel > $K/kernel.asm
	    $(OBJDUMP) -t $K/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $K/kernel.sym





#
########################################################
          # .S -> .o
########################################################
#

# this is to generate the .o file from the .S file
# there is no rule for .c to .o because make have already a default rule for it so we don't
# need to define
# we can see the defualt rules of make via this make -p -f /dev/null
$K/%.o: $K/%.S
		$(CC) -march=rv64gc -g -c -o $@ $<

# just tagging for debugging
tags: $(OBJS)
	etags kernel/*.S kernel/*.c

# .O files are intermediate files, .c --->(compile) .o  ---->(link) final executable file
# so make can delete them after linking process, so to prevent we use the .PRECIOUS
.PRECIOUS: %.o


# in compiler command we have -MD FLAG, it generates the dependency files(.d files) while compiling the .c
# files, let say we have abc.c file which depends on def.c and efg.c file, now by -MD flag we are
# saying that whilc compiling also generate the .d file in which we track this file is 
# depend on which files.

# we do this because if in future we change any dependency file then we want to regenerate the .o
# file for the abc.o, so we do not track that make will not generate the abc.o again if there
# is change in the def.c not abc.c.

# including here means make knows about the .d files
-include kernel/*.d


clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*.d */*.asm */*.sym \
	$K/kernel





#
########################################################
           # GDB Settings
########################################################
#

# try to generate a unique GDB port
# GDB is the debugging tool, and we are setting the port for it
GDBPORT = $(shell expr `id -u` % 5000 + 25000)

# forming the command to start the gdb server
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)



ifndef CPUS
CPUS := 3
endif

# -machine virt means create a virtual machine
# -bios none means there is no bios and -kernel means use this kernel
# -128M = 128 MB RAM, -smp = symmetric multiprocessing which define number of the cpus
# -nographic = do not create graphical display window(GUI) instead we interact via console/terminal
# virtio is for the storage disk
#  
QEMUOPTS = -machine virt -bios none -kernel $K/kernel -m 128M -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
#QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
#QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0


#
########################################################
           # command to create the VM
########################################################
#
qemu: check-qemu-version $K/kernel
	$(QEMU) $(QEMUOPTS)


.gdbinit: .gdbinit.tmpl-riscv
	sed "s/:1234/:$(GDBPORT)/" < $^ > $@

qemu-gdb: $K/kernel .gdbinit fs.img
	@echo "*** Now run 'gdb' in another window." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

print-gdbport:
	@echo $(GDBPORT)

QEMU_VERSION := $(shell $(QEMU) --version | head -n 1 | sed -E 's/^QEMU emulator version ([0-9]+\.[0-9]+)\..*/\1/')
check-qemu-version:
	@if [ "$(shell echo "$(QEMU_VERSION) >= $(MIN_QEMU_VERSION)" | bc)" -eq 0 ]; then \
		echo "ERROR: Need qemu version >= $(MIN_QEMU_VERSION)"; \
		exit 1; \
	fi

.PHONY: fmt
fmt:
	clang-format -i $(wildcard kernel/*.[ch] user/*.[ch] mkfs/*.c)



