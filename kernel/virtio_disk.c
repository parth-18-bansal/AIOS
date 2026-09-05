#include "spinlock.h"
#include "types.h"
#include "memlayout.h"
#include "virtio.h"


#define R(r) ((volatile uint32 *)(VIRTIO0 + (r)))

static struct disk{

    struct virtq_desc *desc;

    struct virtq_avail *avail;

    struct virtq_used *used;

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



}