#include "spinlock.h"

struct {
    struct spinlock lock;

    #define INPUT_BUF_SIZE 128

    char buf[INPUT_BUF_SIZE];

    /*
    buf is a circular array. 

    read index tells from where to start reading
    write index tells upto which index reading is ready
    edit index track the next index at which we have to write the next character

    when we write something via keyboard then it get store in buf array, and e index tracks the next
    index at which we have to store next input character.
    
    now when we enter or EOF(end of file) happens then w gets updated and becomes equal to e. and tell
    upto here we have to read.

    read stores the index from where we have to start reading.
    */
    uint r; // read index
    uint w; // write index
    uint e; // edit index
} cons;

// to get the ascii value of ctr + key
#define C(x) ((x) - '@')



/*
summary:
*/
void consoleintr(int c){
    // c stores the character that we get from keyboard
    switch(c){
        /*
        if ctr + P then give the list of process
        */
        case C('P'):
            procdump();
            break;

        case C('U'):
            while(){

            }
            break;

        default:
            // e-r = input_buf_size means array is full with unread characters
            // c should not be equal to null character
            if(c != 0 && cons.e - cons.r < INPUT_BUF_SIZE){
                // converting carriage return to newline because some terminals use \n and some \r
                c = (c == '\r') ? '\n' : c;


            }
    }
}