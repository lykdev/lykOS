#pragma once

#include <common/sync/spinlock.h>
#include <dev/block.h>
#include <lib/def.h>
#include <lib/errno.h>
#include <lib/list.h>

#define VFS_MAX_NAME_LEN 64

typedef struct vnode vnode_t;

typedef enum
{
    VNODE_REG,
    VNODE_DIR,
    VNODE_FIFO,
    VNODE_CHAR,
    VNODE_BLOCK,
    VNODE_SOCKET
}
vnode_type_t;

typedef struct
{
    int (*open)  (vnode_t *self, const char *name, vnode_t **out);
    int (*close) (vnode_t *self);
    int (*read)  (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
    int (*write) (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
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
    const char *name;

    bool (*probe)(block_device_t *part);
    bool (*get_root_vnode)(block_device_t *part, vnode_t **out);

    list_node_t list_node;
    u64 ref_count;
}
filesystem_type_t;

/*
 * Mount points
 */

int vfs_mount(block_device_t *blk, filesystem_type_t *fs_type, const char *path);

void vfs_register_fs_type(filesystem_type_t *fs_type);

int vfs_open(const char *path, vnode_t **out);

int vfs_close(vnode_t *vn);

int vfs_create(const char *path, vnode_type_t t, vnode_t **out);

int vfs_remove(const char *path);

static inline void VN_HOLD(vnode_t *vn)
{
    spinlock_acquire(&vn->slock);

    vn->ref_count++;

    spinlock_release(&vn->slock);
}

static inline void VN_RELE(vnode_t *vn)
{
    spinlock_acquire(&vn->slock);

    vn->ref_count--;

    if (vn->ref_count == 0)
    {
        vfs_close(vn);
        return;
    }

    spinlock_release(&vn->slock);
}

/*
 * Initialization & Debug
 */

void vfs_init();

void vfs_debug();
