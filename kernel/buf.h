#include "sleeplock.h"
#include "fs.h"

/*
buf is the buffer cache, kernel cache the block of the disk in the RAM, so that
it does not have to access the disk again and again for the data
*/
struct buf{
    int valid; // has data been read from disk?
    uint dev;  // device number
    uint blockno; // disk is divided into multiple blocks so block no
    uint refcnt;  // reference count, how many kernel components are refering it at a time.

    // disk indicate whether disk controller is writing or reading data from the buf or not
    // if disk controller is interacting with that buffer then other process can not use it
    int disk;
    struct sleeplock lock;
    
    /*
    it is used to implement the LRU cache = least recently used cache
    although all the bufs are store in the array, here we also maintain the linkedlist of the buf
    And in the linked list first element is the buf which is used most recently and last
    element is the one which is least recently used(oldest used)

    so it is helpful when buf's array is fully filled and kernel want to cache a new disk block
    then it check which buf is least recently used and then it remove that LRU buf with new
    buf.

    but refcnt must be 0 for replacing an existing buf
    */
    struct buf *prev;
    struct buf *next;

    // in it we store the actual data of the disk block and each block is of size 1024 bytes
    uchar data[BSIZE];
};