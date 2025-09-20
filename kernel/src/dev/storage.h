#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct storage_device storage_device_t;

typedef enum
{
    RAMDISK,
    NETWORK,
    HDD,
    SSD,
    NVME,
}
storage_device_type;

struct storage_device
{
    const char *name;
    storage_device_type type;

    u64 sector_size;

    u64 (*read) (storage_device_t *self, u64 lba, void *buffer, u64 count);
    u64 (*write)(storage_device_t *self, u64 lba, void *buffer, u64 count);
    u64 (*get_sector_count)(storage_device_t *self);
    u64 (*get_free_sector_count)(storage_device_t *self);

    list_t partitions;

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
