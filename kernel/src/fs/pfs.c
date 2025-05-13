#include "pfs.h"

#include <common/assert.h>
#include <common/log.h>
#include <common/hhdm.h>
#include <mm/heap.h>
#include <lib/list.h>
#include <lib/string.h>

static vfs_node_t* lookup(vfs_node_t *self, const char *name);
static const char* list(vfs_node_t *self, u64 *hint);
static vfs_node_t* create(vfs_node_t *self, vfs_node_type_t t, char *name);

typedef struct
{
    vfs_node_t vfs_node;
    list_t children;
    spinlock_t spinlock;
    list_node_t list_node;
}
pfs_node_t;

static vfs_node_ops_t g_node_dir_ops = (vfs_node_ops_t) {
    .lookup = lookup,
    .list = list,
    .create = create
};

static vfs_node_t* lookup(vfs_node_t *self, const char *name)
{
    pfs_node_t *parent_node = (pfs_node_t*)(self);

    FOREACH(n, parent_node->children)
    {
        pfs_node_t *child = LIST_GET_CONTAINER(n, pfs_node_t, list_node);
        if (strcmp(child->vfs_node.name, name) == 0)
            return &child->vfs_node;
    }

    return NULL;
}

static const char* list(vfs_node_t *self, u64 *hint)
{
    pfs_node_t *parent = (pfs_node_t*)(self);

    if (*hint == 0xFFFF)
        return NULL;

    list_node_t *next;
    if (*hint == 0)
        next = parent->children.head;
    else
    {
        // The hint points to a data structure that should be located in the higher half.
        ASSERT(*hint > HHDM);
        next = ((list_node_t*)*hint)->next;
    }

    if (next)
    {
        *hint = (u64)next;
        return LIST_GET_CONTAINER(next, pfs_node_t, list_node)->vfs_node.name;
    }
    else
    {
        *hint = 0xFFFF;
        return NULL;
    }
}

static vfs_node_t* create(vfs_node_t *self, vfs_node_type_t type, char *name)
{
    pfs_node_t *parent_node = (pfs_node_t*)(self);

    spinlock_acquire(&parent_node->spinlock);

    pfs_node_t *new_node = heap_alloc(sizeof(pfs_node_t));
    *new_node = (pfs_node_t) {
        .vfs_node = (vfs_node_t) {
            .type = type,
            .ops = type == VFS_NODE_DIR ? &g_node_dir_ops : NULL
        },
        .children = LIST_INIT,
        .spinlock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy(new_node->vfs_node.name, name);

    list_append(&parent_node->children, &new_node->list_node);

    spinlock_release(&parent_node->spinlock);
    return &new_node->vfs_node;
}

vfs_mountpoint_t *pfs_new_mp(const char *name)
{
    vfs_mountpoint_t *mp = heap_alloc(sizeof(vfs_mountpoint_t));
    pfs_node_t *root_node = heap_alloc(sizeof(pfs_node_t));

    *root_node = (pfs_node_t) {
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
