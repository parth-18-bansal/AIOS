#define BSIZE 1024 //BLOCK SIZE

#define NDIRECT 12

// dirsiz is the length of the filename
// max file name can be 14 char long
#define DIRSIZ 14

/*
Layout of the disk is like this:
Boot block | super block | log | inode blocks | free bit map | data blocks

bit map tracks which data block is free and which data block is empty, here if bit value
is 0 then crossponding datablock is empty and if bit value is 1 then corresponding data block
is filled.

so super block stores the information like where are data blocks where are inodes block etc
*/
struct superblock{
    uint magic;  // it should be equal to the FSMAGIC VALUE
    uint size;  // total number of blocks
    uint nblocks; // number of the data blocks
    uint ninodes;  // number of inodes block
    uint nlog;  // number of log blocks
    uint logstart;  // block number of first log block
    uint inodestart;  // block number of first inode block
    uint bmapstart;  // block number of first free map block
};


#define FSMAGIC 0x10203040

/*
this struct is stored in the disk and inode is the copy of this and inode struct get stored
in the RAM.
*/
struct dinode{
    short type;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
};


/*
IPB = inode per block
so here we are calculating how many inodes are stored in one disk block
so blocksize / size of one inode 
*/
#define IPB (BSIZE / sizeof(struct dinode))


/*
it finds in which disk block ith inode is stored
*/
#define IBLOCK(i, sb) ((i) / IPB + sb.inodestart)

/*
dirent = directory entry.
directory is a file that contains the records(dirent)

and each record / dirent is a pair = (inum, filename)

if we are accessing a file in a directory then kernel
open the directory file, and then search the pair that have
filename = requested file and this is how kernel gets the inode number then kernel
load that inode in the memory from the disk.

this is how kernel gets the data of that file.

*/
struct dirent{
    ushort inum;

    /*
    in c each string should end with the /0 (null char)
    so here we are saying there can be strings which do not have /0 at the end so
    file name can be of 14 char long not 13
    */
    char name[DIRSIZ] __attribute__((nonstring));
};