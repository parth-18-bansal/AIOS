#include "memlayout.h"

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
#define THR 0;

// it is the receive holding register: it stores a byte to read(input)
// when uart gets a input byte then it stores it here and then transfer it to the THR for transmit
#define RHR 0;

// it is interrupt enable register, it means can uart raise interrupt or not
#define IER 1;

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


/*
summary:
*/
void uartintr(void){
    ReadReg(ISR);

    acquire(&tx_lock);
    if(ReadReg(LSR) && LSR_TX_IDLE){

    }
}