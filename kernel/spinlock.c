#include "types.h"
#include "proc.c"


/*
sstatus is a register that stores the status of the supervisor mode
sstatus's value's one bit store the global SIE(supervisor interrupts enabled)
i.e there is sie register where each bit represent on interrupts and we can set 
which interrupt we want to disable and which not, but through this bit of the sstatus
we can disable all the interrupts globally by setting one bit = 0 in the sstatus.

rc = read clear so it means read the value and store it some where then clear the value,
so here we are readng the complete value of the sstatus register and then we are clearing only
SIE bit of it to 0 so disabling the interrupts

after that increasing the count of the noff by one, it counts how many time pushoff is called
it value increase by one whenever a pushoff is called and value decrease by 1 when popoff is called

in intena we are storing the old value of the noff only if noff = 0 (means pushoff is not called yet)


summary: disabling the interrupt globally
*/
void push_off(void){
    uint64 flags = rc_sstatus(SSTATUS_SIE);
    int old = !!(flags & SSTAUS_SIE);

    if (mycpu()->noff == 0){
        mycpu()->intena = old;
    }
    mycpu()->noff += 1;
}