/*
there are three types of files.

T_FILES means it is regular file that contains data. like hello.txt
T_DIR means it is directory file, it data is DIRENT object, so in this data is of type
DIRENT.
T_DEVICE it is device file that represent the driver and hardware device.
we perform operation on the device file, and then kernel uses its major/minor numbers to select
the corresponding driver and then driver performs tha actual operation on the hardware
*/
#define T_DIR 1    // Directory files
#define T_FILE 2    // regular files
#define T_DEVICE 3 //device files