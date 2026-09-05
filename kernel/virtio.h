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

struct virtq_desc{

};

struct virtq_avail{

};

struct virtq_used_elem{

};

struct virtq_used{

};