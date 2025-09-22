#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct storage_device storage_device_t;

typedef enum
{
    STORAGE_DEVICE_RAMDISK,
    STORAGE_DEVICE_NETWORK,
    STORAGE_DEVICE_HDD,
    STORAGE_DEVICE_SSD,
    STORAGE_DEVICE_NVME,
}
storage_device_type;

struct storage_device
{
    const char *name;
    storage_device_type type;
    list_t partitions;

    u64 sector_size;
    u64 sector_count;

    bool (*read) (storage_device_t *self, u64 lba, void *buffer, u64 count);
    bool (*write)(storage_device_t *self, u64 lba, void *buffer, u64 count);

    void *driver_data;

    spinlock_t slock;
    list_node_t list_node;
};

typedef struct
{
    u16 name[36]; // UTF-16 little endian.
    int number;
    u64 lba_start;
    u64 sector_count;

    storage_device_t *parent_device;

    list_node_t list_node;
}
partition_t;

void storage_device_register(storage_device_t *dev);
