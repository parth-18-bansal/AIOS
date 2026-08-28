#include "fs.h"
#include "file.h"
#include "spinlock.h"
#include "param.h"


/*
itable stores the cached inode in RAM that we fetch from the disk
*/
struct{
    struct spinlock lock;
    struct inode inode[NINODE];
} itable;


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

*/
static char *skipelem(){

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