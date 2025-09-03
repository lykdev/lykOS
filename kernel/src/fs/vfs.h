#pragma once

#include <common/sync/spinlock.h>
#include <lib/def.h>
#include <lib/errno.h>

#define VFS_MAX_NAME_LEN 64

typedef struct vnode vnode_t;

typedef enum
{
    VFS_NODE_FILE,
    VFS_NODE_DIR,
    VFS_NODE_FIFO,
    VFS_NODE_CHAR,
    VFS_NODE_BLOCK,
    VFS_NODE_SOCKET
}
vnode_type_t;

typedef struct
{
    int (*open)  (vnode_t *self);
    int (*close) (vnode_t *self);
    int (*read)  (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
    int (*write) (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
    int (*lookup)(vnode_t *self, const char *name, vnode_t **out);
    int (*list)  (vnode_t *self, u64 *hint, const char **out);
    int (*create)(vnode_t *self, char *name, vnode_type_t t, vnode_t **out);
    int (*ioctl) (vnode_t *self, u64 request, void *args);
}
vnode_ops_t;

struct vnode
{
    char name[VFS_MAX_NAME_LEN]; // Filename.
    vnode_type_t type;
    u32 perm;  // Permission mask.
    u32 uid;   // User id.
    u32 gid;   // Group id.
    u64 size;  // File size.
    u64 ctime; // Time created.
    u64 mtime; // Time modified.
    u64 atime; // Time accessed.

    vnode_ops_t *ops;
    void *mp_data;

    spinlock_t slock;
    u64 ref_count;
};

typedef struct
{
    vnode_t *root_node;
}
vfs_mountpoint_t;

/*
 * Veneer layer
 */

int vfs_open(const char *path, vnode_t **out);

int vfs_close(vnode_t *vn);

int vfs_create(const char *path, vnode_type_t t, vnode_t **out);

int vfs_remove(const char *path);

inline void VN_HOLD(vnode_t *vn)
{
    spinlock_acquire(&vn->slock);

    vn->ref_count++;

    spinlock_release(&vn->slock);
}

inline void VN_RELE(vnode_t *vn)
{
    spinlock_acquire(&vn->slock);

    vn->ref_count--;

    spinlock_release(&vn->slock);
}

/*
 * Mount points
 */

 int vfs_mount(const char *path, vfs_mountpoint_t *mp);

/*
 * Initialization & Debug
 */

void vfs_init();

void vfs_debug();
