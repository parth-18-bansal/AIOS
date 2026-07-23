#include "types.h"

// checking interrupt is enable or not
static inline int intr_get(){
    uint64 x = r_sstatus();
    return (x & SSTATUS_SIE) != 0;
}

// enable the interrupt
static inline void intr_on(){
    s_sstatus(SSTATUS_SIE);
}

// disable the interrupt
static inline void intr_off(){
    c_sstatus(STATUS_SIE);
}