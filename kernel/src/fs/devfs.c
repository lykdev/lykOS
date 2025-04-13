#include "devfs.h"

#include <common/log.h>
#include <common/panic.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <mm/heap.h>

static vfs_node_t *nodes[256];

static vfs_node_t *root_create(vfs_node_t*, vfs_node_type_t t, char *name)
{
    if (t != VFS_NODE_CHAR)
        panic("Only creating char inside `/dev` is supported right now!");

    for (int i = 0; i < 256; i++)
        if (nodes[i] == NULL)
        {
            nodes[i] = heap_alloc(sizeof(vfs_node_t));
            nodes[i]->size = 0;
            strcpy(nodes[i]->name, name);
            return nodes[i];
        }

    panic("Maximum number of files under `/dev` reached!");
    return NULL;
}

static vfs_node_t *root_lookup(vfs_node_t*, const char *name)
{
    for (int i = 0; i < 256; i++)
        if (strcmp(name, nodes[i]->name) == 0)
            return nodes[i];

    return NULL;
}

static const char* root_list(vfs_node_t *, uint *index)
{
    return (const char *)&nodes[*index++]->name;
}

static vfs_node_ops_t root_ops = (vfs_node_ops_t) {
    .create = root_create,
    .lookup = root_lookup,
    .list = root_list
};

static vfs_node_t root_node = (vfs_node_t) {
    .type = VFS_NODE_DIR,
    .ops  = &root_ops
};

static vfs_mountpoint_t devfs_mp = (vfs_mountpoint_t) {
    .root_node = &root_node
};

void devfs_init()
{
    vfs_mount("/dev", &devfs_mp);

    log("Mounted devfs at `/dev`.");
}
