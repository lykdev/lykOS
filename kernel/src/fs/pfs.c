#include "pfs.h"

#include <mm/heap.h>
#include <lib/list.h>
#include <lib/string.h>

static vfs_node_t* lookup(vfs_node_t *self, const char *name);
static vfs_node_t* create(vfs_node_t *self, vfs_node_type_t t, char *name);

typedef struct
{
    vfs_node_t vfs_node;
    list_t children;
    spinlock_t spinlock;
    list_node_t list_node;
}
node_t;

static vfs_node_ops_t g_node_ops = (vfs_node_ops_t) {
    .lookup = lookup,
    .create = create
};

static vfs_node_t* lookup(vfs_node_t *self, const char *name)
{
    node_t *parent_node = (node_t*)(self);

    FOREACH(n, parent_node->children)
    {
        node_t *child = LIST_GET_CONTAINER(n, node_t, list_node);
        if (strcmp(child->vfs_node.name, name) == 0)
            return &child->vfs_node;
    }

    return NULL;
}

static vfs_node_t* create(vfs_node_t *self, vfs_node_type_t t, char *name)
{
    node_t *parent_node = (node_t*)(self);
    spinlock_acquire(&parent_node->spinlock);

    node_t *new_node = heap_alloc(sizeof(node_t));
    *new_node = (node_t) {
        .vfs_node = (vfs_node_t) {
            .type = t,
            .ops = &g_node_ops
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
    node_t *root_node = heap_alloc(sizeof(vfs_node_t));

    *root_node = (node_t) {
        .vfs_node = (vfs_node_t) {
            .type = VFS_NODE_DIR,
            .ops = &g_node_ops
        },
        .children = LIST_INIT,
        .spinlock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy(root_node->vfs_node.name, name);

    mp->root_node = &root_node->vfs_node;

    return mp;
}
