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
1) first we are creating buffers of the log blocks
2) we are updated buffers that are already created and updated
3) now we copying the changes from disk blocks buffers to the log blocks buffers
4) writing back those changes to the disk log blocks

so copying the changes from the modified buffers to log blocks

file operation --> executed in buffers --> (here) copying those changes to log blocks
*/
static void write_log(void){
    int tail;

    for(tail = 0; tail < log.lh.n; tail++){
        struct buf *to = bread(log.dev, log.start + tail + 1); // log block
        struct buf *from = bread(log.dev, log.lh.block[tail]); // cache buffer

        memmove(to->data, from->data, BSIZE);

        bwrite(to);
        brelse(from);
        brelse(to);
    }
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

/*
summmary:
copy the log blocks and pasting it into disk blocks like inode, data, bitmaps
*/
static void install_trans(int recovering){
    int tail;

    for(tail = 0; tail < log.lh.n; tail++){
        if(recovering){
            printk("recovering tail %d dst %d\n", tail, log.lh.block[tail]);
        }

        struct buf *lbuf = bread(log.dev, log.start + tail + 1); // log buffer
        struct buf *dbuf = bread(log.dev, log.lh.block[tail]); // disk data, inode ,bitmap buffer

        memmove(dbuf->data, lbuf->data, BSIZE); // copy log block into dst blocks

        bwrite(dbuf);

        // bunpin in recoring = 0 because recovering indicates that whether there is crash or
        // before install_trans

        // and if crash occured then ram is gone so buffer is in ram so buffer is also gone
        // bpin or unpin has no significance in that case.
        if(recovering == 0){
            bunpin(dbuf);
        }

        brelse(lbuf);
        brelse(dbuf);
    }
}

/*
if crash happens then we run this to make sure disk inconsistency don't happen.
*/
static void recover_from_log(void){
    read_head();

    // if committed then copy from the log to disk
    install_trans(1);
    log.lh.n=0;
    write_head();
}

/*
commiting is copying the changes from buffer to log block to actual disk blocks
*/
static void commit(){
    if(log.lh.n > 0){
        write_log();
        write_head();
        install_trans(0);
        log.lh.n = 0;
        write_head();
    }
}

/*
summary:
1) here first we are checking whether commiting is happening or not, if yes the process go
for sleep
2) if number of blocks modified + expected number of blocks that can be modified > maximum
number of log blocks then process go for sleep
3) other wise it just increase outstanding count.
*/
void begin_op(void){
    acquire(&log.lock);

    while(1){
        if(log.committing){
            sleep_prepare(&log);
            release(&log.lock);
            sleep();
            acquire(&log.lock);
        }

        else if(log.lh.n + (log.outstanding + 1)*MAXOPBLOCKS > LOGBLOCKS){
            sleep_prepare(&log);
            release(&log.lock);
            sleep();
            acquire(&log.lock);
        }

        else{
            log.outstanding += 1;
            release(&log.lock);
            break;
        }
    }
}

/*
summary:
it just decrease the outstanding count by 1, and wakeup all process that are sleeping on
log channel
2) and if outstanding count becomes 0 then it call commiting
*/
void end_op(void){
    int do_commit = 0;

    acquire(&log.lock);

    log.outstanding -= 1;
    if(log.committing){
        panic("log.committing");
    }

    if(log.outstanding == 0){
        do_commit = 1;
        log.committing = 1;
    }

    else{
        wakeup(&log);
    }

    release(&log.lock);

    if(do_commit){
        commit();
        acquire(&log.lock);
        log.committing = 0;
        log.ncommit += 1;
        wakeup(&log);
        release(&log.lock);
    }
}

/*
summary:
this increase the count of the log.lh.n if a block buffer is modified also stores block
number.
*/
void log_write(struct buf *b){
    int i;

    acquire(&log.lock);

    // checking if we add new block in the list then is it greater or equal to LOGBLOCKS
    // then do not add new block in the list and give error.
    // it also prevent MAXOPBLOCKS limit crossing
    if(log.lh.n >= LOGBLOCKS){
        panic("too big a transaction");
    }

    /*
    if outstanding is zero it means there is no begin_op() without end_op, but for this
    filesystem operation begin_op has been called so why outstanding is 0 it should atleast 
    equal to 1, it means log_write has been called without begin_op so error.
    */
    if(log.outstanding < 1){
        panic("log_write outside of trans");
    }

    // if a block that is modified is already present in the lh.block list then break
    // because if it is already listed then do not add it again in the array.
    for(i = 0; i < log.lh.n; i++){
        if(log.lh.block[i] == b->blockno){
            break;
        }
    }

    // storing the block number in the lh.block array
    log.lh.block[i] = b->blockno;

    /*
    only increase the size of log.lh.n if new block is added other wise not
    */
    if(i == log.lh.n){
        bpin(b);
        log.lh.n++;
    }

    release(&log.lock);
}