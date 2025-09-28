#include "devfs.h"

#include <fs/vfs.h>
#include <common/assert.h>
#include <common/sync/spinlock.h>
#include <lib/def.h>
#include <lib/errno.h>
#include <lib/list.h>
#include <lib/string.h>
#include <mm/heap.h>

typedef struct
{
    vnode_t vn;



    list_t children;
    list_node_t list_node;
}
devfs_node_t;

static int devfs_open  (vnode_t *self, const char *name, vnode_t **out);
static int devfs_close (vnode_t *self);
static int devfs_read  (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
static int devfs_write (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
static int devfs_list  (vnode_t *self, u64 *hint, const char **out);
static int devfs_create(vnode_t *self, char *name, vnode_type_t t, vnode_t **out);
static int devfs_ioctl (vnode_t *self, u64 request, void *args);

static vnode_ops_t devfs_ops = {
    .open   = devfs_open,
    .close  = devfs_close,
    .read   = devfs_read,
    .write  = devfs_write,
    .list   = devfs_list,
    .create = devfs_create,
    .ioctl  = devfs_ioctl,
};

static int devfs_open(vnode_t *self, const char *name, vnode_t **out)
{
    ASSERT(name && out);

    if (self->type != VNODE_DIR)
    {
        *out = NULL;
        return ENOTDIR;
    }

    devfs_node_t *parent = (devfs_node_t *)self;
    FOREACH(n, parent->children)
    {
        devfs_node_t *child = LIST_GET_CONTAINER(n, devfs_node_t, list_node);
        if (strcmp(child->vn.name, name) == 0)
        {
            VN_HOLD(&child->vn);
            *out = &child->vn;
            return EOK;
        }
    }

    *out = NULL;
    return ENOENT;
}

static int devfs_close(vnode_t *self)
{
    ASSERT(self->ref_count == 0);

    heap_free((devfs_node_t *)self);

    return EOK;
}

static int devfs_read(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{

}

static int devfs_write(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{

}

static int devfs_list(vnode_t *self, u64 *hint, const char **out)
{
    ASSERT(hint && out);

    if (self->type != VNODE_DIR)
    {
        *out = NULL;
        return ENOTDIR;
    }

    devfs_node_t *parent = (devfs_node_t *)self;

    if (*hint == 0xFFFF)
    {
        *out = NULL;
        return EOK;
    }

    list_node_t *next;
    if (*hint == 0)
        next = parent->children.head;
    else
        next = ((list_node_t *)*hint)->next;

    if (next)
    {
        *hint = (u64)next;

        devfs_node_t *child = LIST_GET_CONTAINER(next, devfs_node_t, list_node);
        *out = (const char *)&child->vn.name;
        return EOK;
    }
    else
    {
        *hint = 0xFFFF;
        *out = NULL;
        return EOK;
    }
}

static int devfs_create(vnode_t *self, char *name, vnode_type_t t, vnode_t **out)
{
    ASSERT(name && out);

    devfs_node_t *parent = (devfs_node_t *)self;
    devfs_node_t *child  = heap_alloc(sizeof(devfs_node_t));

    *child = (devfs_node_t) {
        .vn = (vnode_t) {
            .type = t,
            .size = 0,
            .ops  = &devfs_ops,
            .slock = SPINLOCK_INIT,
            .ref_count = 1
        },
        .children = LIST_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy(child->vn.name, name);

    list_append(&parent->children, &child->list_node);

    *out = &child->vn;
    return EOK;
}

static int devfs_ioctl(vnode_t *self, u64 request, void *args)
{
    return ENOTSUP;
}

static devfs_node_t devfs_root_node = {
    .vn = (vnode_t) {
        .name = "/",
        .type = VNODE_DIR,
        .ops = &devfs_ops
    },
    .children = LIST_INIT,
    .list_node = LIST_NODE_INIT
};

void devfs_init()
{
    vfs_mount_vnode(&devfs_root_node.vn, "/dev");
}
