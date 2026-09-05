#include "sleeplock.h"
#include "defs.h"
#include "proc.h"
#include "spinlock.h"


void initsleeplock(struct sleeplock *lk, char *name){
    initlock(&lk->lk, "sleep lock");

    lk->name = name;
    lk->locked = 0;
    lk->pid = 0;
}

/*
summary:
1) first we are acquiring the sleeplock's spinlock because,

there can be two cases:
let say we have a process A and process b, and both tries to acquire the sleep lock then if we
do not add the acquiring the spinlock step then if both acquire sleeplock at a same time then
they can acquire sleep simultaneously.

also if one process is just callling sleep and get interrupt and in the meanwhile other proces
calls wakeup then first process may get sleep forever when it come back.
*/
void acquiresleep(struct sleeplock *lk){
    acquire(&lk->lk);

    while(lk->locked){
        sleep_prepare(lk);
        release(&lk->lk);
        sleep();
        acquire(&lk->lk);
    }

    lk->locked = 1;
    lk->pid = myproc()->pid;

    release(&lk->lk);
}

// releasing sleep lock
void releasesleep(struct sleeplock *lk){
    acquire(&lk->lk);

    lk->locked = 0;
    lk->pid = 0;

    wakeup(lk);
    
    release(&lk->lk);
}

// it tells whether current process is holding the sleep lock or not
int holdingsleep(struct sleeplock *lk){
    int r;

    acquire(&lk->lk);
    r = lk->locked && (lk->pid == myproc()->pid);
    release(&lk->lk);

    return r;
}