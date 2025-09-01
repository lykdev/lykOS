#include "sysfs.h"

#include <common/assert.h>
#include <common/log.h>
#include <common/hhdm.h>
#include <mm/heap.h>
#include <lib/list.h>
#include <lib/string.h>

// Inode defintion.

typedef struct
{
    vnode_t  vfs_node;
    list_t      children;
    spinlock_t  spinlock;
    list_node_t list_node;
}
inode_t;

// Forward declarations.

static int file_read(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
static int file_write(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);

static int dir_lookup(vnode_t *self, const char *name, vnode_t **out);
static int dir_list(vnode_t *self, u64 *hint, const char **out);
static int dir_create(vnode_t *self, char *name, vnode_type_t t, vnode_t **out);

//

static vnode_ops_t g_node_file_ops = {
    .read = file_read,
    .write = file_write,
};

static vnode_ops_t g_node_dir_ops = {
    .lookup = dir_lookup,
    .list = dir_list,
    .create = dir_create
};

//

static int file_read(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{
    if (!self->mp_data || offset >= self->size)
    {
        *out = 0;
        return EOK;
    }

    if (offset + count > self->size)
        count = self->size - offset;

    memcpy(buffer, self->mp_data + offset, count);
    *out = 0;
    return EOK;
}

static int file_write(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{
    if (!self->mp_data)
    {
        *out = 0;
        return EOK;
    }

    if (offset + count > self->size)
    {
        // Expand buffer area.
        u64 new_size = offset + count;
        self->mp_data = heap_realloc(self->mp_data, self->size, new_size);
        self->size = new_size;
    }

    memcpy(self->mp_data + offset, buffer, count);
    *out = count;
    return EOK;
}

static int dir_lookup(vnode_t *self, const char *name, vnode_t **out)
{
    inode_t *parent_node = (inode_t*)(self);

    FOREACH(n, parent_node->children)
    {
        inode_t *child = LIST_GET_CONTAINER(n, inode_t, list_node);
        if (strcmp(child->vfs_node.name, name) == 0)
        {
            *out = &child->vfs_node;
            return EOK;
        }
    }

    *out = NULL;
    return -ENOENT;
}

static int dir_list(vnode_t *self, u64 *hint, const char **out)
{
    inode_t *parent = (inode_t*)(self);

    if (*hint == 0xFFFF)
    {
        return EOK;
        *out = NULL;
    }

    list_node_t *next;
    if (*hint == 0)
        next = parent->children.head;
    else
    {
        // The hint points to a data structure that should (obv) be located in the higher half.
        ASSERT(*hint > HHDM);
        next = ((list_node_t*)*hint)->next;
    }

    if (next)
    {
        *hint = (u64)next;
        *out = LIST_GET_CONTAINER(next, inode_t, list_node)->vfs_node.name;
        return EOK;
    }
    else
    {
        *hint = 0xFFFF;
        *out = NULL;
        return EOK;
    }
}

static int dir_create(vnode_t *self, char *name, vnode_type_t t, vnode_t **out)
{
    inode_t *parent_node = (inode_t*)(self);

    spinlock_acquire(&parent_node->spinlock);

    inode_t *new_node = heap_alloc(sizeof(inode_t));
    *new_node = (inode_t) {
        .vfs_node = (vnode_t) {
            .type  = t,
            .perm  = 0x777,
            .uid   = 0,
            .gid   = 0,
            .size  = 0,
            .ctime = 0,
            .mtime = 0,
            .atime = 0
        },
        .children = LIST_INIT,
        .spinlock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy(new_node->vfs_node.name, name);

    vnode_t *vn = &new_node->vfs_node;
    switch (t)
    {
        case VFS_NODE_FILE:
            vn->ops = &g_node_file_ops;
            break;
        case VFS_NODE_DIR:
            vn->ops = &g_node_dir_ops;
            break;
        case VFS_NODE_FIFO:
        case VFS_NODE_SOCKET:
        case VFS_NODE_CHAR:
        case VFS_NODE_BLOCK:
            vn->ops = NULL;
            break;
        default:
            panic("Unknown VFS node type: %d!", t);
    }

    list_append(&parent_node->children, &new_node->list_node);

    spinlock_release(&parent_node->spinlock);
    *out = &new_node->vfs_node;
    return EOK;
}

//

vfs_mountpoint_t *sysfs_new_mp(const char *name)
{
    vfs_mountpoint_t *mp = heap_alloc(sizeof(vfs_mountpoint_t));
    inode_t *root_node = heap_alloc(sizeof(inode_t));

    *root_node = (inode_t) {
        .vfs_node = (vnode_t) {
            .type = VFS_NODE_DIR,
            .ops = &g_node_dir_ops
        },
        .children = LIST_INIT,
        .spinlock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy(root_node->vfs_node.name, name);

    mp->root_node = &root_node->vfs_node;
    return mp;
}
