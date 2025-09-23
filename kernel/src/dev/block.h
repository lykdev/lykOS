#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct block_device block_device_t;

typedef enum
{
    BLOCK_DEVICE_DRIVE,
    BLOCK_DEVICE_PARTITION
}
block_device_type;

typedef bool (*block_device_read_t) (block_device_t *self, u64 lba, void *buffer, u64 count);
typedef bool (*block_device_write_t)(block_device_t *self, u64 lba, void *buffer, u64 count);
typedef int  (*block_device_ioctl_t)(block_device_t *self, u64 request, void *args);

struct block_device
{
    const char *name;
    block_device_type type;

    list_t partitions;  // List of child partitions (for drives).
    block_device_t *parent; // Parent device (for partitions).

    u64 lba_offset;   // Starting LBA offset (for partitions).
    u64 sector_size;  // Size of each sector in bytes.
    u64 sector_count; // Total number of sectors.

    block_device_read_t  read;
    block_device_write_t write;
    block_device_ioctl_t ioctl;

    void *driver_data; // Driver-specific private data.

    spinlock_t slock;
    list_node_t list_node;
    u64 ref_count;
};

void block_device_register_drive(
    const char *name,
    u64 sector_size,
    u64 sector_count,
    block_device_read_t read,
    block_device_write_t write,
    void *driver_data
);
