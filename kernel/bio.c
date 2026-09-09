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
1) here first we are traversing the bcache and finding if buffer already exist or not
2) if we do not find the buffer then we are creatng a new buffer buffer by overwriting
the buffer which is not in use and is least recently used.
*/
static struct buf *bget(uint dev, uint blockno){
    struct buf *b;

    acquire(&bcache.lock);

    // if buffer is in the buffer cache
    for(b = bcache.head.next; b != &bcache.head; b = b->next){
        if(b->dev = dev && b->blockno == blockno){
            b->refcnt++;
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }

    // if required buffer is not in the buffer cache
    // here we over write the buffer which is not in use and also is least recently used
    for(b = bcache.head.prev; b!=&bcache.head; b = b->prev){
        if(b->refcnt == 0){
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0;
            b->refcnt = 1;
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }

    panic("bget: no buffers");
}

/*
summary:
1) here bread get the dev and blockno then it calls the bget to get the corresponding buffer
2) if buffer does not data copied from the disk then it calls the virtio_disk_rw, which creates
the request and send to disk for data and then data get copied to the buffer from the disk
3) at last it returns the buffer
*/
struct buf *bread(uint dev, uint blockno){
    struct buf *b;

    b = bget(dev, blockno);

    if(!b->valid){
        virtio_disk_rw(b,0);
        b->valid = 1;
    }

    return b;
}

/*
summary:
*/
brelse(struct buf *b){
    if(!holdingsleep(&b->lock)){
        panic("brelse");
    }

    releasesleep(&b->lock);

    acquire(&bcache.lock);

    b->refcnt--;

    if(b->refcnt == 0){

        /*
        here if refcnt == 0 then we will remove the buffer cache element and placed
        it in the front of the bcache linked list after the head element
        */
        b->next->prev = b->prev;
        b->prev->next = b->next;

        b->next = bcache.head.next;
        b->prev = &bcache.head;

        bcache.head.next->prev = b;
        bcache.head.next = b;
    }

    release(&bcache.lock);
}