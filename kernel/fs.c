#include "fs.h"
#include "file.h"
#include "spinlock.h"
#include "param.h"
#include "sleeplock.h"


struct superblock sb;


/*
itable stores the cached inode in RAM that we fetch from the disk
*/
struct{
    struct spinlock lock;
    struct inode inode[NINODE];
} itable;

/*
summary:
*/
void iinit(){
    int i = 0;

    initlock(&itable.lock, "itable");

    for(i = 0; i < NINODE; i++){
        initsleeplock(&itable.inode[i].lock, "inode");
    }
}

static struct inode *iget(uint dev, uint inum);


struct inode *idup(struct inode *ip){
    acquire(&itable.lock);

    ip->ref++;

    release(&itable.lock);

    return ip;
}

/*
summary:
1) traverse inode array(in memory cache of inodes) and check whether we have requested inode
or not.
2) if yes then returns that inode and if no then check whether there is any empty element in the 
array or not
3) if there is no space for new inode then gives error otherwise create new inode entry
in the array and return that inode address

iget() only set the dev,inum, ref,valid attributes in the new inode element, other things
are copied by ilock()
*/
static struct inode * iget(uint dev, uint inum){
    struct inode *ip, *empty;

    acquire(&itable.lock);

    empty = 0;

    for(ip=&itable.inode[0]; ip < &itable.inode[NINODE]; ip++){
        if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
            ip->ref++;
            release(&itable.lock);
            return ip;
        }

        if(empty == 0 && ip->ref == 0){
            empty = ip;
        }
    }

    if(empty == 0){
        panic("iget: no inodes");
    }

    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->valid = 0;
    ip->ref = 1;

    release(&itable.lock);

    return ip;
};

/*
summary:
this function is just to acquire the lock of the inode. and if inode does not have the disk
dinode data then we copies the data from the disk to the buffer then to the inode(RAM).
*/
void ilock(struct inode *ip){
    struct buf *bp;
    struct dinode *dip;

    if(ip == 0 || ip->ref < 1){
        painc("ilock");
    }

    acquiresleep(&ip->lock);

    /*
    if inode has not been copied in the ram from the disk then it will first read the disk
    block and cached it in the buffer then copied the values of the dinode into the inode.
    */
    if(ip->valid == 0){
        // here we are getting the buffer which cached the disk block were that inode is stored
        bp = bread(ip->dev, IBLOCK(ip->inum, sb));

        // inum % ipb tells the position of the requried inode in that data block.
        dip = (struct dinode *)bp->data + ip->inum % IPB;

        ip->type = dip->type;
        ip->major = dip->major;
        ip->minor = dip->minor;
        ip->nlink = dip->nlink;
        ip->size = dip->size;

        memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));

        brelse(bp);

        ip->valid = 1;

        if(ip->type == 0){
            panic("ilock: no type");
        }
    }
}

/*
summary:
skipelem = skip element
it is used to extract the path element from the path and return the remaining path

for example /usr/bin/ls
then path elements are the usr, bin, ls

then we extract the first path element and stores it in the name, so name = usr

and return the remaing path = bin/ls

*/
static char *skipelem(char *path, char *name){
    char *s;
    int len;

    while(*path == "/"){
        path++;
    }

    if(*path == 0){
        return 0;
    }

    s = path;
    
    while(*path != "/" && *path != 0){
        path++;
    }

    len = path - s;

    if(len>=DIRSIZ){
        memmove(name, s, DIRSIZ);
    }
    else{
        memmove(name, s, len);
        name[len] = 0;
    }

    while(*path == '/'){
        path++;
    }

    return path;
}

/*
summary:
*/
static struct inode *namex(){

}

/*
summary:
*/
struct inode *namei(){
    char name[DIRSIZ];

}