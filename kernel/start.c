// decalaring the function main
void main();

void start(){
    // mstatus is the cpu register that stores the status of the cpu, value is of 64 bit
    // r_mstatus is reading the value of the mstatus register
    // mstatus is the type the CSR registers, there are different type of the registers 
    // in the cpu like general purpose registers or control and status registers, so mstatus belong to CSR family
    unsigned long x = r_mstatus();

    // MSTATUS stores the 64 bit value, in which each bit represent
    // something about the cpu state so nth bit represent the MPP
    // MPP is machine previous privilege which define the privilege mode of the cpu
    // from the super visor mode, machine mode, user mode
    // so here by bit masking we are only changing the MPP bit and setting it to the 
    // supervisor mode from the machine mode 
    x &= ~MSTATUS_MPP_MASK;
    x |= MSTATUS_MPP_S;

    // updating the value of the mstatus register
    w_mstatus(x);

    // mepc is also a register belong to the CSR family, it stores the program counter
    // value. so when machine go to the supervisor mode from the machine mode by mret
    // the program counter register value will be equal to the mepc value
    // program counter register tell the cpu address of the instruction to be executed

    // here we are setting the value of the mepc to the main function
    w_mepc((uint64)main);

    // satp is the csr register that stores the address of the root page table
    // satp = supervisor address translation and protection.
    // here value of the satp's top bit represent paging table disable or not.
    // at the time of the booting there is no kernel page table so we are setting the 
    // page table on/off bit to 0(means disable paging)
    // so satp = 0 tells MMU that paging is disabled and access the things using the
    // physical address.
    w_satp(0);

    // there are exceptions and interrupts and in starting these exceptions and interrupts are
    // handle by the machine mode, means if any exception or interrupt occur then cpu mode get change
    // to the machine mode, here we are delegating 15 exceptions and interrupt to the supervisor
    // mode, means now if any exc or inter will happen among those 15 exceptions then supervisor 
    // mode will handle it. here 0xffff means 15 times 1, and if bit is 0 then it means that 
    // exception is handle by the machine mode and if bit is 1 then that exception is handle by 
    // the supervisor mode. and each bit represents a unique exception or interrupt
    // medeleg or mideleg are csr registers and we are setting there value.
    w_medeleg(0xffff);
    w_mideleg(0xffff);

    // delegate means who will handle that interrupt or exception, but sie tells that
    // does supervisor mode is allowed to accept that interrupt or not
    // so in sie that interrupt is disabled then supervisor mode will get the interrupt but
    // supervisor mode will not handle it. sie is the csr register that stores which interrupt 
    // supervisor mode can handle here we are doing the bit masking setting only interested bits
    // sie = supervisor interrupt enable register
    w_sie(r_sie() | SIE_SEIE | SIE_STIE);

    
}