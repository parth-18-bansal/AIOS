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



