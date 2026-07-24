#include "types.h"
#include "proc.c"
#include "riscv.h"
#include "spinlock.h"

/*
summary: it disable the interrupt because it is accessing the shared kernel
memory which proc's spinlock's locked field, so interrupt should not reschedule the cpu
after acquring the lock and before releasing the lock.
And then we are just setting the value of the locked field to on(1).
*/
void acquire(struct spinlock *lk){
    push_off();
    if(holding(lk)){
        panic("aquire");
    }

    /*
    here we are just exchanging the value that is locked will be equal to the 1
    and it will return the old value. Also __ATOMIC_ACQUIRE will confirm that
    the instruction after this statement must run after successfully running of
    this statement, means cpu and compiler sometime to optimisation the execution
    reorder the code instruction if output remain same after reordering so this thing
    will say to the cpu and compiler to not reorder and execute instruction after this statement before it
    */
    while(__atomic_exchange_n(&lk->locked, 1, __ATOMIC_ACQUIRE) != 0);

    lk->cpu = mycpu();
}

/*
summary: just changing the locked value to 0 and decreasing the value of the noff by 1.
*/
void release(struct spinlock *lk){
    if(!holding(lk)){
        panic("release");
    }

    lk->cpu = 0;
    
    /*
    setting the value of the locked to 0 and here __ATOMIC_RELEASE make sure that
    all the instruction before this statement should run before the execution of this statement
    reording of the instruction should be such that the instruction defined before this not execute
    after it.
    */
    __atomic_store_n(&lk->locked, 0, __ATOMIC_RELEASE);

    pop_off();


}


/*
summary: check does locked is enabled or not
*/
int holding(struct spinlock *lk){
    int r;
    r = (lk -> locked && lk->cpu == mycpu());
    return r;
}


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

/*
summary: if noff become 1 before running this then enable the interrupts other wise decrease
the noff value by 1
*/
void pop_off(void){
    struct cpu *c = mycpu();

    if(intr_get()){
        panic("pop_off - interruptible");
    }

    if(c->noff < 1){
        panic("pop_off");
    }

    c->noff -= 1;

    if(c->noff = 0 && c->intena){
        intr_on();
    }
}