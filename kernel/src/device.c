#include "dev.h"

#include <common/log.h>
#include <lib/string.h>

static device_t *devices[256];
static int dev_cnt = 0;

static int root_lookup(vfs_node_t*, char *name, vfs_node_t **out)
{
    for (int i = 0; i < dev_cnt; i++)
        if (strcmp(name, devices[i]->vfs_node.name) == 0)
        {
            *out = &devices[i]->vfs_node;
            return 0;
        }  

    *out = NULL;
    return 0;
}

// static int root_list(vfs_node_t *self, uint *index, char **out)
// {
//     return 0;
// }

static vfs_node_ops_t root_ops = (vfs_node_ops_t) {
    .read = NULL,
    .write = NULL,
    .lookup = root_lookup,
    .list = NULL,
    .ioctl = NULL
};

static vfs_node_t root_node = (vfs_node_t) {
    .type = VFS_NODE_DIR,
    .name = "dev",
    .ops  = &root_ops
};

static vfs_mountpoint_t devfs_mp = (vfs_mountpoint_t) {
    .root_node = &root_node
};

void dev_init()
{
    vfs_mount("/dev", &devfs_mp);
}

void device_register(device_t *dev)
{
    devices[dev_cnt++] = dev;
    log("Registered device `%s`.", dev->vfs_node.name);
}