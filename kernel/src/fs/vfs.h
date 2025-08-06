#pragma once

#include <common/sync/spinlock.h>
#include <lib/def.h>

#define VFS_MAX_NAME_LEN 64

typedef struct vfs_node vfs_node_t;

typedef enum
{
    VFS_NODE_FILE,
    VFS_NODE_DIR,
    VFS_NODE_FIFO,
    VFS_NODE_CHAR,
    VFS_NODE_BLOCK,
    VFS_NODE_SOCKET
}
vfs_node_type_t;

typedef struct
{
    u64 (*read) (vfs_node_t *self, u64 offset, void *buffer, u64 count);
    u64 (*write)(vfs_node_t *self, u64 offset, void *buffer, u64 count);
    vfs_node_t* (*lookup)(vfs_node_t *self, const char *name);
    const char* (*list)  (vfs_node_t *self, u64 *hint);
    vfs_node_t* (*create)(vfs_node_t *self, vfs_node_type_t t, char *name);
    int (*ioctl)(vfs_node_t *self, u64 request, void *args);
}
vfs_node_ops_t;

struct vfs_node
{
    char name[VFS_MAX_NAME_LEN]; // Filename.
    vfs_node_type_t type;
    u32 perm;  // Permission mask.
    u32 uid;   // User id.
    u32 gid;   // Group id.
    u32 size;  // File size.
    u64 ctime; // Time created.
    u64 mtime; // Time modified.
    u64 atime; // Time accessed.

    vfs_node_ops_t *ops;

    spinlock_t lock;
    u16 ref_count;
    void *mp_data;
};

typedef struct
{
    vfs_node_t *root_node;
}
vfs_mountpoint_t;

int vfs_mount(const char *path, vfs_mountpoint_t *mp);

vfs_node_t *vfs_create(vfs_node_type_t t, const char *path);

vfs_node_t *vfs_lookup(const char *path);

void vfs_init();

void vfs_debug();
