#pragma once

#include "dev/block.h"
#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct storage_device storage_device_t;

typedef enum
{
    STORAGE_DEVICE_RAM,
    STORAGE_DEVICE_NET,
    STORAGE_DEVICE_HDD,
    STORAGE_DEVICE_SSD,
    STORAGE_DEVICE_NVME
}
storage_device_type_t;

struct storage_device
{
    const char *name;
    storage_device_type_t type;
    list_t partitions;

    block_device_t *blk;

    void *driver_data;

    spinlock_t slock;
    list_node_t list_node;
    u64 ref_count;
};

typedef struct
{
    const char *name;

    storage_device_t *parent;

    block_device_t *blk;

    spinlock_t slock;
    list_node_t list_node;
    u64 ref_count;
}
storage_partition_t;

bool storage_scan_partitions(storage_device_t *dev);

extern list_t g_storage_device_list;
extern spinlock_t g_storage_device_slock;
