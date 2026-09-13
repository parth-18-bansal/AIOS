// DEFINING THE MACROS
#define NPROC 64  // max num of processes
#define NCPU 8    // number of cpus

#define NINODE 50 // maximum number of active inodes that can be cached in the RAM at a time

#define MAXOPBLOCKS 10  // it is the maximum number of blocks one operation is allowed to modify
#define LOGBLOCKS (MAXOPBLOCKS * 3) // it is maximum modified blocks the entire log can hold at a time
#define NBUF (MAXOPBLOCKS * 3) // SIZE OF THE buf cache
