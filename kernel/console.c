#include "spinlock.h"
#include "spinlock.h"
#include "defs.h"

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
        // move the cursor one step backward
        uartputc_sync('\b');
        // overwrite the existing with space
        uartputc_sync(' ');
        // again move the cursor one step backward
        uartputc_sync('\b');
    }
    else{
        uartputc_sync(c);
    }
}


/*
summary: so it copy the bytes from the buffer array to the user-space buffer, this is how a process
read the input from keyboard.

conputc and uartputc_sync, printk echo the input to screen

so input is going to places, screen and process that want to read it
*/
int consoleread(int user_dst, uint64 dst, int n){
    uint target;

    target = n;
    int c;
    char cbuf;

    acquire(&cons.lock);

    while(n > 0){
        // here we are waiting for interrupt handler to put some uncommited new input in buffer
        while(cons.r == cons.w){
            if(killed(myproc())){
                release(&cons.lock);
                return -1;
            }

            sleep_prepare(&cons.r);
            release(&cons.lock);
            sleep();
            acquire(&cons.lock);
        }


        c = cons.buf[cons.r++ % INPUT_BUF_SIZE];

        /*
        here if c is end of file then we are breaking but we are decreasing read index and again pointing
        it to ctrl + D, because if again process read then it get ctrl + D and second time read()
        return with 0.
        */
        if(c == C('D')) {
            if(n < target){
                cons.r--;
            }
            break;
        }

        cbuf = c;

        // copy the input byte(1 byte) to user-space buffer.
        if(either_copyout(user_dst, dst, &cbuf, 1) == -1){
            break;
        }

        dst++;
        --n;

        if(c == '\n'){
            // if whole line is copied from buffer to user buffer then return to user-level read()
            // it can be possibel we want to read target bytes but if only read n bytes because
            // \n next line come
            break;
        }

    }

    release(&cons.lock);

    return target - n;
}


/*
summary:
write() syscall will call this, then this will stores the bytes into buf and then
uartwrite will store bytes into uart chip from buf, from where bytes can go to different places
like stdout etc.

here we are transfering data from a program's memory to the destination.

so user_src = to check whether we are transfering data from user space memory(user program)
or kernel program
src = starting address from where we have to transfer the data, 
n = number of bytes
*/
int consolewrite(int user_src, uint64 src, int n){
    // we will transfer the data in a batch of 32 bytes
    char buf[32];
    int i = 0;

    while(i < n){
        int nn = sizeof(buf);
        
        /*
        if number of bytes to transfer are less then 32 bytes then nn = n - i
        */
        if(nn > n-i){
            nn = n-i;
        }

        // transfering the byte to the buf array.        
        if(either_copyin(buf, user_src, src+1, nn) == -1){
            break;
        }

        uartwrite(buf,nn);

        i = i + nn;
    }
}



/*
summary: 
console intr stores the input character in the buffer array and when there is enter, end of file or
if buffer arrays gets full with unread characters then it wake up consoleread() function.

and if there is ctrl + U, ctrl + H, backspace then it erase the character

and if ctrl + P then it prints the list of running processes
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

        // ctrl + U kill the line means erase complete line
        // here we are moving the e index back to index that is after /n
        // and in screen we are erasing all the chracter that are after /n(enter) i.e characters
        // between w and e index in array. and moving back the cursor to index that is after /n.
        case C('U'):
            while(cons.w != cons.e && cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
                cons.e--;
                consputc(BACKSPACE);
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