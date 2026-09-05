#include "spinlock.h"
#include "types.h"
#include "memlayout.h"
#include "virtio.h"
#include "riscv.h"
#include "fs.h"
#include "buf.h"


#define R(r) ((volatile uint32 *)(VIRTIO0 + (r)))

static struct disk{

    struct virtq_desc *desc;

    struct virtq_avail *avail;

    struct virtq_used *used;

    struct {
        struct buf *b;
        char status;
    } info[NUM];

    struct virtio_blk_req ops[NUM];

    // here we are keeping the track of whether a descriptor is free or not
    char free[NUM];

    struct spinlock vdisk_lock;
} disk;


/*
summary:
*/
void virtio_disk_init(void){
    uint32 status = 0;

    initlock(&disk.vdisk_lock, "virtio_disk");


    /*
    here we are checking whether the connected device is a virtio or not and also 
    checking it's version, device_id and vendor id.
    */
    if (*R(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
      *R(VIRTIO_MMIO_VERSION) != 2 || *R(VIRTIO_MMIO_DEVICE_ID) != 2 ||
      *R(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551) {
         panic("could not find virtio disk");
    }

    /*
    here we are setting the status in the status register of the virtio disk*/

    // reset device
    *R(VIRTIO_MMIO_STATUS) = status;

    // here this status means we acknowledge that device is vitio disk
    // set ACKNOWLEDGE status bit
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    *R(VIRTIO_MMIO_STATUS) = status;

    // here we are saying virtio_disk.c is driver of the vitio_disk
    // set DRIVER status bit
    status |= VIRTIO_CONFIG_S_DRIVER;
    *R(VIRTIO_MMIO_STATUS) = status;

    // disabling some features
    uint64 features = *R(VIRTIO_MMIO_DEVICE_FEATURES);

    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_FLUSH);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);

    *R(VIRTIO_MMIO_DRIVER_FEATURES) = features;

    // tell device that feature negotiation is complete.
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    *R(VIRTIO_MMIO_STATUS) = status;

    // confirming featurs setting is done or not
    status = *R(VIRTIO_MMIO_STATUS);
    if (!(status & VIRTIO_CONFIG_S_FEATURES_OK))
    panic("virtio disk FEATURES_OK unset");

    // here SEL = SELECTING we are selecting the queue, it is not selecting actually
    // we defining the queue with id 0, there can be multiple queue
    *R(VIRTIO_MMIO_QUEUE_SEL) = 0;

    // ensure queue 0 is not in use.
    if (*R(VIRTIO_MMIO_QUEUE_READY))
    panic("virtio disk should not be ready");

    /*
    max queue size depends on the device configuration, here we are only checking
    whether size select by us is smaller then max or not
    */
    uint32 max = *R(VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (max == 0){
        panic("virtio disk has no queue 0");
    }
    if (max < NUM){
        panic("virtio disk max queue too short");
    }

    /*
    there are three things:
    descriptor, available ring, used ring
    a) in descriptor instructions are written, which device/disk controller will
    fetch and execute. it is queue
    b) available ring: it is like a list of pending instuction to execute. so each
    descriptor has a number, and descriptors exists as chain(linked list), so available
    page stores the head of each chain that is pending.
    c) used page: it stores the completed instructions, so it stores head of the completed
    descriptor chain.
    */
    disk.desc = kalloc();
    disk.avail = kalloc();
    disk.used = kalloc();
    if (!disk.desc || !disk.avail || !disk.used)
        panic("virtio disk kalloc");
    memset(disk.desc, 0, PGSIZE);
    memset(disk.avail, 0, PGSIZE);
    memset(disk.used, 0, PGSIZE);

    // set queue size.
    *R(VIRTIO_MMIO_QUEUE_NUM) = NUM;

    /*
    address of the used, available, desc page is of 64 bit length
    but registers in the virtio disk are of 32 bit size, so to store the address of each page
    we need two register pair. so LOW register stores the lowest 32 bit of the address and 
    HIGH register stores the top 32 bit of the address
    */

    // queue is just a memory page.

    *R(VIRTIO_MMIO_QUEUE_DESC_LOW) = (uint64)disk.desc;
    *R(VIRTIO_MMIO_QUEUE_DESC_HIGH) = (uint64)disk.desc >> 32;
    *R(VIRTIO_MMIO_DRIVER_DESC_LOW) = (uint64)disk.avail;
    *R(VIRTIO_MMIO_DRIVER_DESC_HIGH) = (uint64)disk.avail >> 32;
    *R(VIRTIO_MMIO_DEVICE_DESC_LOW) = (uint64)disk.used;
    *R(VIRTIO_MMIO_DEVICE_DESC_HIGH) = (uint64)disk.used >> 32;

    // queue is ready.
    *R(VIRTIO_MMIO_QUEUE_READY) = 0x1;

    for(int i = 0; i<NUM; i++){
        disk.free[i] = 1;
    }

    // tell device we're completely ready.
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    *R(VIRTIO_MMIO_STATUS) = status;
}

/*
summary:
disk.free array keep track of the discriptor which are free.
so here we are traversing that array and if we found a discriptor which is free then
we are returning it's number.
*/
static int alloc_desc(){
    for(int i = 0; i < NUM; i++){
        if(disk.free[i]){
            disk.free[i] = 0;
            return i;
        }
    }

    return -1;
}

// here we are free the desc by setting it's value 0 and also marking it free in the
// free array
static void free_desc(int i){
    if (i >= NUM){
        panic("free_desc i");
    }
    if(disk.free[i]){
        panic("free_desc 2");
    }

    disk.desc[i].addr = 0;
    disk.desc[i].len = 0;
    disk.desc[i].flags = 0;
    disk.desc[i].next = 0;

    disk.free[i] = 1;
    
    /*
    this wakeup call run everytime to check if any process gets sleep because
    of zero free descriptor then wakeup that process 
    */
    wakeup(&disk.free[0]);
}

/*
summary:
here we allocating 3 descriptor per request
1st desc = virtio blk req
2nd desc = data buffer specify the ram buffer(disk cache)
3rd desc = status
*/
static int alloc3_desc(int *idx){
    for(int i = 0; i<3; i++){
        idx[i] = alloc_desc();

        if(idx[i] < 0){
            for(int j = 0; j<i; j++){
                free_desc(idx[j]);
            }
            return -1;
        }
    }

    return 0;
}


void virtio_disk_rw(struct buf *b, int write){
    /*
    we can visualise disk division in terms of sectors or blocks

    os divides disk in terms of block and disk divides itself in terms of sector
    each sector is of 512 bytes of length and block is of 1024 bytes of length

    so one block = 2 sectors so BSIZE / 512 = 2 so here we are finding which sector
    to read/write with the help of block number. 
    */
    uint64 sector = b->blockno * (BSIZE / 512);

    // disk struct is common struct for all cpus
    acquire(&disk.vdisk_lock);

    int idx[3];
    while(1){
        if(alloc3_desc(idx) == 0){
            break;
        }

        // sleeps if there is no free descriptor
        sleep_prepare(&disk.free[0]);
        release(&disk.vdisk_lock);
        sleep();
        acquire(&disk.vdisk_lock);
    }



    // creating the block request struct
    struct virtio_blk_req *buf0 = &disk.ops[idx[0]];

    if(write){
        buf0->type = VIRTIO_BLK_T_OUT; // write the disk
    }
    else{
        buf0->type = VIRTIO_BLK_T_IN;  // read the disk
    }

    buf0->sector = sector;
    buf0->reserved = 0;

    // then setting the value of the descriptor chain's head equal to the block request
    disk.desc[idx[0]].addr = (uint64)buf0;
    disk.desc[idx[0]].len = sizeof(struct virtio_blk_req);
    disk.desc[idx[0]].flags = VRING_DESC_F_NEXT;
    disk.desc[idx[0]].next = idx[1];

    // defining second descriptor
    disk.desc[idx[1]].addr = (uint64)b->data;
    disk.desc[idx[1]].len = BSIZE;

    /*
    if we have to write into the disk then it means we are copying the data from the ram
    buffer to the disk, so it means disk device will read from the ram buffer that is why 
    we set the write bit of the flag as 0 if write is true.

    and if we are reading from the disk it means we are copying data from the disk and 
    writing it into the ram buffer so we are setting the desc.flag = write
    */
    if(write){
        disk.desc[idx[1]].flags = 0; // device read b->data
    }
    else{
        disk.desc[idx[1]].flags = VRING_DESC_F_WRITE; //device write into b->data
    }

    disk.desc[idx[1]].flags |= VRING_DESC_F_NEXT;
    disk.desc[idx[1]].next = idx[2];

    disk.info[idx[0]].status = 0xff; // here if value is 0 then it means success
    
    disk.desc[idx[2]].addr = (uint64)&disk.info[idx[0]].status;
    disk.desc[idx[2]].len = 1;
    disk.desc[idx[2]].flags = VRING_DESC_F_WRITE;
    disk.desc[idx[2]].next = 0;

    
}