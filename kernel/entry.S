# Assembly code
# this code will run for each cpu core individually, and here we are assigning a
# a stack to each cpu, stack is used to run the nested functions of the kernel code, to
# to start the kernel and run its code first each cpu core need a stack so here we assiging that
# stack. Now in kernel we are following a convention where stack will grow downward means the address
# of the first element is greater than the address of the next element in the stack


# so here we are assigning the address of the starting pointer of the stack to sp register
_entry:
    # load the address of the stack0 label into the sp register
    la sp, stack0

    # load immediate the constant into the a0 register
    li a0, 1024*4

    # cssr is the csr read, so it means read the value from the
    # csr registers(CPU REGISTER) and store the value into the a0 register
    # hartid is the cpu core number
    # csr = control and status register
    # mhartid is a register belongs to the CSR family and it stores the hartid
    csrr a1, mhartid

    # addi = add immediate
    addi a1, a1, 1

    mul a0, a0, a1
    add sp, sp, a0
    
    # jump to the start function of start.c
    call start
