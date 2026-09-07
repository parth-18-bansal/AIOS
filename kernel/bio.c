#include <spinlock.h>
#include <buf.h>
#include <param.h>


/*
summary: this is the buffer cache
*/
struct{
    struct spinlock lock;
    struct buf buf[NBUF];
    
    struct buf head;

} bcache;

/*
summary:
here we are creating a linkedlist of buffers

head --> n --> n-1 --> ... --> 1 --> head
*/
void binit(void){
    struct buf *b;

    initlock(&bcache.lock, "bcache");

    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;

    for(b = bcache.buf; b < bcache.buf + NBUF; b++){
        b->next = bcache.head.next;
        b->prev = &bcache.head;

        initsleeplock(&b->lock, "buffer");

        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
}

/*
summary:
*/
static struct buf *bget(){
}

/*
summary:
*/
struct buf *bread(){
}