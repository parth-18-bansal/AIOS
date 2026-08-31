/*
spin lock are for short term locks means the lock that are held for short time
while sleep locks are for situation where we have to held for long time

so I/O from disk is a long process that is why there we will use the sleep lock

in spin lock, if lock is already acquired by a process then another process continously try
to acquire the lock due to while loop so cpu is continously in use by that process.

but in sleeplock if any process is already acquiring the lock and new process tries to aquire the
same lock then if lock is held then that new process go to the sleep state.
*/
struct sleeplock {
    uint locked;

    struct spinlock lk;

    char *name;
    int pid;
};