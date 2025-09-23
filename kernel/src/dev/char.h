#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct char_device char_device_t;

typedef bool (*char_device_read_t) (char_device_t *self, u64 lba, void *buffer, u64 count);
typedef bool (*char_device_write_t)(char_device_t *self, u64 lba, void *buffer, u64 count);
typedef int  (*char_device_ioctl_t)(char_device_t *self, u64 request, void *args);

struct char_device
{
    const char *name;

    void *driver_data;

    char_device_read_t  read;
    char_device_write_t write;
    char_device_ioctl_t ioctl;

    spinlock_t slock;
    list_node_t list_node;
    u64 ref_count;
};

void char_device_register(
    const char *name,
    char_device_read_t read,
    char_device_write_t write,
    void *driver_data
);
