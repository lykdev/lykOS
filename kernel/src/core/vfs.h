#pragma once

#include <lib/def.h>
#include <lib/list.h>

#define VFS_PATH_SEPARATOR '/'

typedef enum
{
    VFS_NODE_FILE,
    VFS_NODE_DIR
} vfs_node_type_t;

typedef struct vfs_mountpoint
{
    char dev[32];
    char fs_type[32];
    char mount_path[64];

    list_node_t list_element;
} vfs_mountpoint_t;

typedef struct vfs_node
{
    vfs_node_type_t type;

    char name[64];
    u32  size;
    u32  permissions;
    
    u64  creation_time;
    u64  modification_time;
    u64  access_time;

    void *data;
} vfs_node_t;

vfs_mountpoint_t* vfs_mount(const char *dev, const char *fs_type, const char *path);

vfs_mountpoint_t* vfs_get_mountpoint(const char *path);

vfs_node_t* vfs_get_node(const char *path);

void vfs_init();

void vfs_debug_info();
