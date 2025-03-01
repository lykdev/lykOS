#include "devfs.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <lib/def.h>
#include <sys/dev/device.h>

static int root_node_read(vfs_node_t *, u64, u64, void *) { return -1; }

static int root_node_write(vfs_node_t *, u64, u64, void *) { return -1; }

static int root_node_lookup(vfs_node_t *, char *name, vfs_node_t **out)
{
    FOREACH(n, g_device_list)
    {
        device_t *dev = LIST_GET_CONTAINER(n, device_t, list_elem);

        if (dev->devfs_node != NULL)
        {
            *out = dev->devfs_node;
            return 0;
        }
    }

    *out = NULL;
    return 0;
}

static int root_node_list(vfs_node_t *, uint *index, char **out)
{
    ;
    ;
}

// clang-format off

vfs_node_ops_t g_root_node_ops = (vfs_node_ops_t) {
    .read = root_node_read,
    .write = root_node_write,
    .lookup = root_node_lookup,
    .list = root_node_list
};

vfs_node_t g_root_node = (vfs_node_t) {
    .type = VFS_NODE_DIR,
    .name = "dev",
    .ops = &g_root_node_ops
};

vfs_mountpoint_t g_mp = (vfs_mountpoint_t) {
    .root_node = &g_root_node
};

// clang-format on

void devfs_init()
{
    vfs_mount("/dev", &g_mp);

    log("Devfs initialized and mounted.");
}
