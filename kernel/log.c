#include "spinlock.h"
#include "param.h"
#include "fs.h"
#include "buf.h"

/*
log header stores two thing
number of the blocks that has to commit
and what are there disk block numbers

let say 3 file system operation executed, and they changes the buffer of 6 disk block

so n = 6 and block array will store the block number of those blocks
*/
struct logheader{
    int n;
    int block[LOGBLOCKS];
};

struct log{
    struct spinlock lock;

    // start stores the first log block number in the disk
    int start;

    // this store the device number
    int dev;

    /*
    outstanding tracks the number of the fs sys calls that are in execution
    
    things work like this, whenever there is syscall releated to filesystem operation, 
    then we first call the bread and get the disk block in the in memory buffer cache, now
    we apply the changes in this buffer.

    now when outstanding gets 0, commit happens

    commit is the three step process, in which 
    1) first write_log get execute which copy in memory buffer to the log block.
    2) write_head(), copies the log header to the disk
    3) install_trans(), it copies the on-disk log content to the respective data, inode, bitmap blocks
    4) then again write_head() which write the n=0(header = 0) in the disk, means transaction complete

    outstanding increase by 1 if any process call begin_ops and decrease if any process calls end_ops
    */
    int outstanding;
    
    // it is a flag that indicates that commit is happening, during commiting no file operation
    // get execute and process trying to call filesystem operation get into sleep state.
    int committing;
    
    int ncommit; // it counts the number commit operation that has been executed

    struct logheader lh; 
};

struct log log;

void initlog(int dev, struct superblock *sb){
    /*
    logheader should be exactly of one block size
    */
    if(sizeof(struct logheader) >= BSIZE){
        panic("initlog: too big logheader");
    }

    initlock(&log.lock, "log");

    log.start = sb->logstart;
    log.dev = dev;
}

/*
summary:
here we are reading the log header from the disk's log header and copying it into
in-memory log header( that is log header defined in the log struct)
*/
static void read_head(){
    struct buf *buf = bread(log.dev, log.start);

    struct logheader *lh = (struct logheader*)(buf->data);

    int i;
    log.lh.n = lh->n;

    for (i = 0; i < log.lh.n; i++){
        log.lh.block[i] = lh->block[i];
    }

    brelse(buf);
}
/*
summary:
here, 
1)first we read the log header block and create a buffer of it.
2)we copy the changes of the log.logheader into that buffer
3) we writing those changes into the actual log header disk block using bwrite()
*/
static void write_head(void){
    // first create the buffer of the log header block: first block in the log region
    struct buf *buf = bread(log.dev, log.start);

    struct logheader *hb = (struct logheader *)(buf->data);

    int i;

    hb->n = log.lh.n;
    for(i = 0; i < log.lh.n; i++){
        hb->block[i] = log.lh.block[i];
    }

    bwrite(buf);
    brelse(buf);
}