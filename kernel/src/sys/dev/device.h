#pragma once

#include <fs/vfs.h>
#include <lib/def.h>
#include <lib/list.h>
#include <sys/dev/driver.h>

typedef enum
{
    DEVICE_PORTIO,
    DEVICE_PCI
} device_type_t;

typedef struct
{
    u16 segment, bus, slot, func;
} pci_device_t;

typedef struct
{
    device_type_t type;
    vfs_node_t *devfs_node;
    union
    {
        pci_device_t pci;
    };
    list_node_t list_elem;
} device_t;

extern list_t g_device_list;
