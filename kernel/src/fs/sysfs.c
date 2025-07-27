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
    vfs_node_t  vfs_node;
    list_t      children;
    spinlock_t  spinlock;
    list_node_t list_node;
}
inode_t;

// Forward declarations.

static u64 file_read(vfs_node_t *self, u64 offset, void *buffer, u64 count);
static u64 file_write(vfs_node_t *self, u64 offset, void *buffer, u64 count);

static vfs_node_t* dir_lookup(vfs_node_t *self, const char *name);
static const char* dir_list(vfs_node_t *self, u64 *hint);
static vfs_node_t* dir_create(vfs_node_t *self, vfs_node_type_t t, char *name);

//

static vfs_node_ops_t g_node_file_ops = {
    .read = file_read,
    .write = file_write,
};

static vfs_node_ops_t g_node_dir_ops = {
    .lookup = dir_lookup,
    .list = dir_list,
    .create = dir_create
};

static vfs_node_ops_t g_node_fifo_ops = {
    .read = file_read,
    .write = file_write,
};

//

static u64 file_read(vfs_node_t *self, u64 offset, void *buffer, u64 count)
{
    if (!self->mp_data || offset >= self->size)
        return 0;

    if (offset + count > self->size)
        count = self->size - offset;

    memcpy(buffer, self->mp_data + offset, count);
    return count;
}

static u64 file_write(vfs_node_t *self, u64 offset, void *buffer, u64 count)
{
    if (!self->mp_data)
        return 0;

    if (offset + count > self->size)
    {
        // Expand buffer area.
        u64 new_size = offset + count;
        self->mp_data = heap_realloc(self->mp_data, self->size, new_size);
        self->size = new_size;
    }

    memcpy(self->mp_data + offset, buffer, count);
    return count;
}

static vfs_node_t* dir_lookup(vfs_node_t *self, const char *name)
{
    inode_t *parent_node = (inode_t*)(self);

    FOREACH(n, parent_node->children)
    {
        inode_t *child = LIST_GET_CONTAINER(n, inode_t, list_node);
        if (strcmp(child->vfs_node.name, name) == 0)
            return &child->vfs_node;
    }

    return NULL;
}

static const char* dir_list(vfs_node_t *self, u64 *hint)
{
    inode_t *parent = (inode_t*)(self);

    if (*hint == 0xFFFF)
        return NULL;

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
        return LIST_GET_CONTAINER(next, inode_t, list_node)->vfs_node.name;
    }
    else
    {
        *hint = 0xFFFF;
        return NULL;
    }
}

static vfs_node_t* dir_create(vfs_node_t *self, vfs_node_type_t type, char *name)
{
    inode_t *parent_node = (inode_t*)(self);

    spinlock_acquire(&parent_node->spinlock);

    inode_t *new_node = heap_alloc(sizeof(inode_t));
    *new_node = (inode_t) {
        .vfs_node = (vfs_node_t) {
            .type  = type,
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

    vfs_node_t *vn = &new_node->vfs_node;
    switch (type)
    {
        case VFS_NODE_FILE:
            vn->ops = &g_node_file_ops;
            break;
        case VFS_NODE_DIR:
            vn->ops = &g_node_dir_ops;
            break;
        case VFS_NODE_FIFO:
            vn->ops = &g_node_fifo_ops;
            break;
        case VFS_NODE_SOCKET:
        case VFS_NODE_CHAR:
        case VFS_NODE_BLOCK:
            vn->ops = NULL;
            break;
        default:
            panic("Unknown VFS node type: %d!", type);
    }

    list_append(&parent_node->children, &new_node->list_node);

    spinlock_release(&parent_node->spinlock);
    return &new_node->vfs_node;
}

//

vfs_mountpoint_t *sysfs_new_mp(const char *name)
{
    vfs_mountpoint_t *mp = heap_alloc(sizeof(vfs_mountpoint_t));
    inode_t *root_node = heap_alloc(sizeof(inode_t));

    *root_node = (inode_t) {
        .vfs_node = (vfs_node_t) {
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
