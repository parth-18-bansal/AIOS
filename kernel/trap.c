#include "types.h"
#include "proc.h"
#include "defs.h"
#include "riscv.h"


/*
summary:

*/
uint64 usertrap(void){

    int which_dev = 0;

    /*
    here we are checking whether user trap occured or kernel trap, it user trap occur then
    only we implement the usertrap not for the kernel trap
    */
    if((r_sstatus() & SSTATUS_SPP) != 0){
        panic("usertrap: not from user mode");
    }

    /*
    here we chang the stvec to the kernelvec because now we are kernel and executing the kernel code
    to handle the usertrap, now also a trap can occur, and to handle the kernel trap we use the 
    kernelvec that is why we change the stvec to kernelvec otherwise if any kerenl trap occurs then
    uservec will get implement
    */
    w_stvec((uint64)kernelvec);

    struct proc *p = myproc();

    p->trapframe->epc = r_sepc();

    /*
    8 means cause of the trap is system call
    */
    if(r_scause == 8){
        // syscall

        if(killed(p)){
            kexit(-1);
        }

        /*
        we are adding 4 in the epc value because right now sepc stores the address of the instruction
        where trap has occured that is ecall instruction and ecall is of 4 bytes length
        like now we have already called the syscall so we should set the address of the next instruction
        in the epc not syscall other wise it will infinitely execute the ecall
        */
        p->trapframe->epc += 4;

        intr_on();

        syscall();
    }
    else if(){} // device interrupt

    else if(){} // page fault
    
    else{
        printk("usertrap(): unexpected scause 0x%1x pid=%d\n", r_scause(), p->pid);
        
        /*
        stval = supervisor trap value register
        it stores the extra information about the trap

        so sepc = stores the program counter of the user process(instruction where trap happened)
           stval = extra information about the trap
           scause = cause of the trap
        */
        printk("            sepc=0x%1x stval=0x%1x\n", r_sepc(), r_stval());
        setkilled(p);
    }

    if(killed(p)){
        kexit(-1);
    }

    // switch the pagetable to the user process page table
    uint64 satp = MAKE_SATP(p->pagetable);

    return satp;


}