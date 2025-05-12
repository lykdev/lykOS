#pragma once

#include <common/sync/spinlock.h>
#include <lib/def.h>

#define VFS_MAX_NAME_LEN 64

typedef struct vfs_node vfs_node_t;
typedef struct vfs_mountpoint vfs_mountpoint_t;
typedef struct vfs_node_ops vfs_node_ops_t;

struct vfs_mountpoint
{
    vfs_node_t *root_node;
};

typedef enum
{
    VFS_NODE_FILE,
    VFS_NODE_DIR,
    VFS_NODE_CHAR,
    VFS_NODE_BLOCK
}
vfs_node_type_t;

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

    void *mp_data;
};

struct vfs_node_ops
{
    u64 (*read)(vfs_node_t *self, u64 offset, void *buffer, u64 count);
    u64 (*write)(vfs_node_t *self, u64 offset, void *buffer, u64 count);
    vfs_node_t* (*lookup)(vfs_node_t *self, const char *name);
    const char* (*list)(vfs_node_t *self, uint *index);
    vfs_node_t* (*create)(vfs_node_t *self, vfs_node_type_t t, char *name);
};

int vfs_mount(const char *path, vfs_mountpoint_t *mp);

vfs_node_t *vfs_lookup(const char *path);

void vfs_init();

void vfs_debug();
