#pragma once

#include <utils/def.h>
#include <utils/slock.h>

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
    VFS_NODE_DIR
}
vfs_node_type_t;

struct vfs_node
{   
    vfs_node_type_t type;
    vfs_node_ops_t *ops;
    void *mp_node;
};

struct vfs_node_ops
{
    char name[128]; // Filename.
    u32  perm;      // Permission mask.
    u32  uid;       // User id.
    u32  gid;       // Group id.
    u32  size;      // File size.
    u64  ctime;     // Time created.
    u64  mtime;     // Time modified.
    u64  atime;     // Time accessed.

    int (*get_size)(vfs_node_t *self, char **name);
    int (*read)(vfs_node_t *self, u64 offset, u64 size, void *buffer);
    int (*write)(vfs_node_t *self, u64 offset, u64 size, void *buffer);
    int (*lookup)(vfs_node_t *self, char *name, vfs_node_t **out);
    int (*list)(vfs_node_t *self, uint *index, char **out);
};

int vfs_mount(const char *path, vfs_mountpoint_t *mp);

int vfs_lookup(const char *path, vfs_node_t **out);

void vfs_init();

void vfs_debug();
