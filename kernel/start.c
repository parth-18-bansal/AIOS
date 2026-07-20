// Register
// mstatus, mepc, medeleg, mideleg, satp, sie, pmpaddr, pmpcnf

/*
summary
1) here we are manily running the mret instruction to come out from the machine mode
2) so we want to go to the super visor mode and want to assign some privileges
like handing exception interrupts, giving permission of the memory
3) also after completion of this we want to execute the main function in S mode
*/


// decalaring the function main
void main();

void start(){
    // mstatus is the cpu register that stores the status of the cpu, value is of 64 bit
    // r_mstatus is reading the value of the mstatus register
    // mstatus is the type the CSR registers, there are different type of the registers 
    // in the cpu like general purpose registers or control and status registers, so mstatus belong to CSR family
    unsigned long x = r_mstatus();

    /*
    MSTATUS stores the 64 bit value, in which each bit represent
    something about the cpu state so nth bit represent the MPP
    MPP is machine previous privilege which define the privilege mode of the cpu
    from the super visor mode, machine mode, user mode
    so here by bit masking we are only changing the MPP bit and setting it to the 
    supervisor mode from the machine mode
    */
    x &= ~MSTATUS_MPP_MASK;
    x |= MSTATUS_MPP_S;

    // updating the value of the mstatus register
    w_mstatus(x);

    /*
    mepc is also a register belong to the CSR family, it stores the program counter
    value. so when machine go to the supervisor mode from the machine mode by mret
    the program counter register value will be equal to the mepc value
    program counter register tell the cpu address of the instruction to be executed

    here we are setting the value of the mepc to the main function
    */
    w_mepc((uint64)main);

    /*
    satp is the csr register that stores the address of the root page table
    satp = supervisor address translation and protection.
    here value of the satp's top bit represent paging table disable or not.
    at the time of the booting there is no kernel page table so we are setting the 
    page table on/off bit to 0(means disable paging)
    so satp = 0 tells MMU that paging is disabled and access the things using the
    physical address.
    */
    w_satp(0);

    /*
    there are exceptions and interrupts and in starting these exceptions and interrupts are
    handle by the machine mode, means if any exception or interrupt occur then cpu mode get change
    to the machine mode, here we are delegating 15 exceptions and interrupt to the supervisor
    mode, means now if any exc or inter will happen among those 15 exceptions then supervisor 
    mode will handle it. here 0xffff means 15 times 1, and if bit is 0 then it means that 
    exception is handle by the machine mode and if bit is 1 then that exception is handle by 
    the supervisor mode. and each bit represents a unique exception or interrupt
    medeleg or mideleg are csr registers and we are setting there value.
    */
    w_medeleg(0xffff);
    w_mideleg(0xffff);

    // delegate means who will handle that interrupt or exception, but sie tells that
    // does supervisor mode is allowed to accept that interrupt or not
    // so in sie that interrupt is disabled then supervisor mode will get the interrupt but
    // supervisor mode will not handle it. sie is the csr register that stores which interrupt 
    // supervisor mode can handle here we are doing the bit masking setting only interested bits
    // sie = supervisor interrupt enable register
    w_sie(r_sie() | SIE_SEIE | SIE_STIE);



    // pmp = physical memory protection, it is used to control who can access particular region
    // of the memory and what permission it have like read, write, execute
    // for defining this thing we need start, end address of the memory and config
    // we define this information using two registers pmpaddr is use to define the end and start addr
    // and pmpcnf is use to define the config. btw there are multiple pmpaddr and pmpcfg regisers
    // like pmpaddr0, pmpaddr1, pmpmaddr2, pmpcfg0, pmpcfg2 etc
    // so here we are setting supervisor mode has access of the complete memory.
    w_pmpaddr0(0x3fffffffffffffull);
    w_pmpcfg0(0xf);
    
    // menvcfg is csr register. it is machine environment configuration register
    // each entry of the page table is of 64 bit and here we are using the 27 bits for the 
    // PPN(physical page number) and other bits are either not used or use for some flags
    // in that there are two bits for A and D flag, A flag represents accessed, which means
    // does the physical page/frame is accessed(read, write, execute) by the cpu after loading that page from for example
    // disk to the RAM, so 0 means not accessed and one means accessed. And D flag means Dirty flag
    // does that page is modified or not after loading from the disk to the RAM. 

    // it is used for example let say a memory is evicted so if os has to remove some page then it will check
    // whether D is 0 or 1 if it is 0 then it means content of the page in the RAM is same as 
    // disk so it can remove it directly or if D is 1 then it means that page is modified
    // so before removing copy the changes to the disk.

    // here we are doing bit masking and changing only config related to the A and D bits of the
    // page table, so each bit of the menvcfg register's value represent a config so
    // we are changing only config related to the a and d bit

    // so config that we are setting is that if any page is accessed or modified that
    // change the A and D flag bits automatically, so now cpu will change it automatically
    // we do not have to write instructions manually to change it
    w_menvcfg(r_menvcfg() | MENVCFG_ADUE);

    // hartid = hardware thread(cpu number) id
    // mhartid is the csr register m stands for the machine mode, so this register can be accessible 
    // in machine mode, so we are storing the cpu number in the tp(thread pointer) register
    // so that we can know the cpu number in the supervisor mode easily.
    int id = r_mhartid();
    w_tp(id);

    // if we want to run the assembly code in the C then we use the asm, mret machine mode return
    // here m means this instruction can be run in the machine mode and it and ret means return
    // where we will return depends on the MPP value.
    asm volatile("mret");
    
}