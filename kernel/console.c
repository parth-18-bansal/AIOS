#include "spinlock.h"

struct {
    struct spinlock lock;
} cons;

// to get the ascii value of ctr + key
#define C(x) ((x) - '@')


/*
summary:
*/
void consoleintr(int c){
    switch(c){
        /*
        if ctr + P then give the list of running process
        */
        case C('P'):
            procdump();
            break;
    }
}