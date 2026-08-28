struct inode {
    /*
    inum is the unique number that is used to find the particular inode in the disk
    */
   uint inum;

   // device number
   uint dev;

   /*
   ref count the number of kernel references
   inode is in-memory copy of the inode(dinode) stored in the disk, and different 
   kernel code can reference it at a time, so ref count the number reference
   to the given inode. forexample if any kernel code call ip = iget(inum), then we
   have one reference of the inode that is "ip"

   so if ref is 0 then we can delete that inode in memory copy from the ram.
   */
   int ref;

   /*
   valid tells whether we have copied the inode from the disk or not
   here we have two functions iget() and ilock(), iget() creates in-memory
   inode and sets inum, dev but other attributes are fetched
   and copied by ilock from the disk so before ilock valid attribute's value
   is 0 after that 1.
   */
   int valid;

   // size of the file
   uint size;

   /*
   number of the hardlinks

   Hard link:
        a.txt ──┐
                ├──-> same inode
        b.txt ──┘
   nlink = 2
   */
   short nlink;

   /*
   NDIRECT = 12 SO addrs has 13 elements
   here NDIRECT = number of the direct address

   so there are 12 direct address and one indirect address, direct address
   tells the direct block number where data of the file is stored, where as
   indirect address tell the address of the block which contains the address
   of the data blocks.

   so each block is of the size 1024 bytes, let says file is of size 20000 bytes
   so first 12 element of the addrs arrays stores the 12 data block number where
   data is stored and 13 element's is the address of the block which stores
   the remaining data block number.
   */
   uint addrs[NDIRECT + 1];
};