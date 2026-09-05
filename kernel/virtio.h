// this is the special value that tells particular device is virtio device or not
#define VIRTIO_MMIO_MAGIC_VALUE		0x000 // 0x74726976

#define VIRTIO_MMIO_VERSION		0x004 // version; should be 2
#define VIRTIO_MMIO_DEVICE_ID		0x008 // device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_VENDOR_ID		0x00c // 0x554d4551

#define VIRTIO_MMIO_STATUS		0x070 // read/write


// Device features tell which features virtio supports
// driver features tell which features driver want to use.
#define VIRTIO_MMIO_DEVICE_FEATURES	0x010
#define VIRTIO_MMIO_DRIVER_FEATURES	0x020


#define VIRTIO_CONFIG_S_ACKNOWLEDGE 1
#define VIRTIO_CONFIG_S_DRIVER      2
#define VIRTIO_CONFIG_S_DRIVER_OK   4
#define VIRTIO_CONFIG_S_FEATURES_OK 8

// device feature bits
#define VIRTIO_BLK_F_RO             5  /* Disk is read-only */
#define VIRTIO_BLK_F_SCSI           7  /* Supports scsi command passthru */
#define VIRTIO_BLK_F_FLUSH          9  /* Cache flush command supported */
#define VIRTIO_BLK_F_CONFIG_WCE     11 /* Writeback mode available in config */
#define VIRTIO_BLK_F_MQ             12 /* support more than one vq */
#define VIRTIO_F_ANY_LAYOUT         27
#define VIRTIO_RING_F_INDIRECT_DESC 28
#define VIRTIO_RING_F_EVENT_IDX     29

// queue
#define VIRTIO_MMIO_QUEUE_SEL		0x030 // select queue, write-only
#define VIRTIO_MMIO_QUEUE_READY		0x044 // ready bit
#define VIRTIO_MMIO_QUEUE_NUM_MAX	0x034 // max size of current queue, read-only
#define VIRTIO_MMIO_QUEUE_NUM		0x038 // size of current queue, write-only
#define VIRTIO_MMIO_QUEUE_DESC_LOW	0x080 // physical address for descriptor table, write-only
#define VIRTIO_MMIO_QUEUE_DESC_HIGH	0x084
#define VIRTIO_MMIO_DRIVER_DESC_LOW	0x090 // physical address for available ring, write-only
#define VIRTIO_MMIO_DRIVER_DESC_HIGH	0x094
#define VIRTIO_MMIO_DEVICE_DESC_LOW	0x0a0 // physical address for used ring, write-only
#define VIRTIO_MMIO_DEVICE_DESC_HIGH	0x0a4


/*
NUM defines the number of the descriptors at a time in a queue
*/
#define NUM 8

// flags
#define VRING_DESC_F_NEXT  1 // chained with another descriptor
#define VRING_DESC_F_WRITE 2 // device writes (vs read)

struct virtq_desc{
    uint64 addr;  // address of the virtio_blk_req struct, or ram buffer, status
    uint32 len;   // number of bytes to read/write there
    /*
    it tells that is there any next descriptor chained to it or not.
    also can we write in the addr  or not
    */
    uint16 flags; 
    uint16 next; // next descriptor in the descriptor chain.

};

struct virtq_avail{

};

struct virtq_used_elem{

};

struct virtq_used{

};


#define VIRTIO_BLK_T_IN  0 // read the disk
#define VIRTIO_BLK_T_OUT 1 // write the disk

/*
each complete request has 3 descriptors in a chain
a) first descriptor refers the address of the virtio_blk_req struct, this vitio_blk_req
tells which block of the disk and whether to read or write
b) seconds descriptor refers the address of the buffer(disk buffer where we are caching the disk data)
c) status of the request

so let say we have to read from the disk
then 1st descriptor tells read a x block from the disk, then 2nd descriptor tells the address
of the buffer where we have to cache the read data. then after successful completion of the request
the status will get stored in ram whose address is defined in the 3rd descriptor
*/

// vitio_blk_req is of 16 byte size: 4bytes+4bytes+8bytes
struct virtio_blk_req {
  uint32 type; // VIRTIO_BLK_T_IN or ..._OUT
  uint64 sector;
  // reseved is just unused attribute not in use
  uint32 reserved;
};