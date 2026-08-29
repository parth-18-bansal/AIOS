#define BSIZE //BLOCK SIZE

#define NDIRECT 12

// dirsiz is the length of the filename
// max file name can be 14 char long
#define DIRSIZ 14

/*
this struct is stored in the disk and inode is the copy of this and inode struct get stored
in the RAM.
*/
struct dinode{
    short type;
    short nlink;
    uint size;
    uint addrs[NDIRECT + 1];
}

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