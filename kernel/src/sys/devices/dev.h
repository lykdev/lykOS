#pragma once

#include <fs/vfs.h>
#include <lib/list.h>

typedef enum
{   
    DEV_CHAR,
    DEV_BLOCK
}
dev_type_t;

typedef struct
{
    dev_type_t type;
    void *data;

    vfs_node_t *dev_ds_node_t;
}
dev_t;

extern list_t g_dev_device_list;
