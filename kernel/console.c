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

#define BACKSPACE 0x100


/*
summary:

*/
void consputc(int c){
    if(c == BACKSPACE){

    }
    else{
        uartputc_sync(c);
    }
}



/*
summary:
*/
void consoleintr(int c){
    acquire(&cons.lock);

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

        /*
        Ctrl + H is backspace and \x7f is also backspace and for both cases we are running shared action
        block
        */
        case C('H'): 
        case '\x7f':
            /*
            here we are checking whether there is something to backspace, because 
            w means upto this index character are commited and e represents the index that we have
            written but not commited

            character is commited means kernel can now read the input and work on that input
            so if input is commited it means kernel has read it and worked on it now we can't 
            erase the character that kernel already read it or processing it.

            w means upto this point kernel has read the input and kernel read when we press enter(\n)
            or end of file or if buffer is full

            if there is nothing to erase then we will skip the block.
            */
            if(cons.w != cons.e){
                cons.e--;
                consputc(BACKSPACE);
            }
            break;

        default:
            // e-r = input_buf_size means array is full with unread characters
            // c should not be equal to null character
            if(c != 0 && cons.e - cons.r < INPUT_BUF_SIZE){
                // converting carriage return to newline because some terminals use \n and some \r
                c = (c == '\r') ? '\n' : c;

                //echo back to the user
                consputc(c);

                // store the input character in buf array
                /*
                e++ means first e%size then e++, post increment
                e will increase even after it reaches size of array, but buf is a circular array
                so we are storing the value by doing this e%size, so even if e's value gets increase
                more than size of array the e%size will remain between 0 and size of array.
                */
                cons.buf[cons.e++ % INPUT_BUF_SIZE] = c;

                /*
                if c is next line or end of file(ctr + D is end of file) or 
                buffer gets full, then we will wake up consoleread()
                */
                if(c == '\n' || c == C('D') || cons.e - cons.r == INPUT_BUF_SIZE){
                    // setting the value w which tells upto which point kernel can read
                    cons.w = cons.e;
                    wakeup(&cons.r);
                }
            }
            break;
    }

    release(&cons.lock);
}