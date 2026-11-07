#include "memlayout.h"
#include "defs.h"



/*
reg = register
uart chip has various registers so we set/get the values of those registers using load/store 
instructions and in load/store instruction we use the UART0 + nbyte memory address, then
due to the address checker and data bus that instruction reached the respective register and
uart chip.
*/

// it only give the address of a uart's register
// here UART0 is the starting address and reg is the byte after that.
#define Reg(reg) ((volatile unsigned char*) (UART0 + (reg)))

// here we are writing in the uart's registers
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

// it read the value of a uart's register
#define ReadReg(reg) (*(Reg(reg)))

// it is the transmit holding register: it stores a byte to transmit(output)
#define THR 0

// it is the receive holding register: it stores a byte to read(input)
// when uart gets a input byte then it stores it here and then transfer it to the THR for transmit
#define RHR 0

// it is interrupt enable register, it means can uart raise interrupt or not
#define IER 1

// it means does uart can interrupt when there a value get stored in the RHR
#define IER_RX_ENABLE (1 << 0)

// it means can uart interrupt when a value get stored in the THR
#define IER_TX_ENABLE

// ISR = interrupt status register
// it stores the reason of the interrupt like if data input happen then uart stores a value in it
// on reading that value we can know why interrupt occured
#define ISR 2

// line status register, it stores the status of the uart chip, 
// if THR is empty then it is able to store the transmit data so then nth bit will be zero
// and by that we get to know that THR is empty, lly if data is ready in the RHR then a particular
// bit is 1 etc.
#define LSR 5

// means if 5th bit in LSR is 1 then it means THR is empty
#define LSR_TX_IDLE (1 << 5)

// means if 0th bit in the LSR is 1 then data is read in the RHR register to transmit.
#define LSR_RX_READY (1<<0)


static struct spinlock tx_lock;

// is uart busy sending?
static int tx_busy;

// &tx_chan is th wait channel
static int tx_chan;


/*
summary:
*/
void uartwrite(char buf[], int n){
    int i = 0;
    while(i < n){
        WriteReg(THR, buf[i]);
        i += 1;
    }
}


/*
summary: 
uart is chip, in this chip there is a register RHR.

when we write something in terminal which is running QEMU, then QEMU will store
that we wrote (a byte at a time) in RHR and 0th bit of LSR is change to 1

now uartintr calls uartgetc which reads the value that QEMU stored at RHR and 
return it.
*/
static int uartgetc(void){
    // is input ready to read
    // checking the status of RHR by LSR's 0th bit, if 0th bit is 0 then not ready and if 1 then ready
    if(ReadReg(LSR) & LSR_RX_READY){
        // returns RHR register's value
        return ReadReg(RHR);
    }
    else{
        return -1;
    }
}


/*
summary: it is uart interrupt handler, it will run if we type 
anything(giving input). or if THR get empty.
*/
void uartintr(void){
    ReadReg(ISR); // acknowledge the interrupt

    /*
    THR stores a byte to transmit, if THR is busy and any process wants to 
    transmit, then that process will go for sleep at &tx_chan and when THR
    gets empty then uartintr will get call and this block will wake all the 
    processes that are sleeping at &tx_chan*/
    acquire(&tx_lock);
    if(ReadReg(LSR) && LSR_TX_IDLE){
        tx_busy = 0;
        wakeup(&tx_chan);
    }
    release(&tx_lock);
    
    // whenever we type something(input) then this will run
    // although it will run in transmitting case also but in that case 
    // does not impact anything
    while (1){
        int c = uartgetc();
        if(c == -1){
            break;
        }
        consoleintr(c);
    }
}