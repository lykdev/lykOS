#include <common/log.h>
#include <sys/streams/streams.h>

static int read(vfs_node_t *self, u64 offset, u64 count, void *buffer) { return -1; }

static int write(vfs_node_t *self, u64 offset, u64 count, void *buffer)
{
    log("%s", buffer);
    return 0;
}

static int lookup(vfs_node_t *self, char *name, vfs_node_t **out) { return -1; }

static int list(vfs_node_t *self, uint *index, char **out) { return -1; }

static vfs_node_ops_t node_ops = (vfs_node_ops_t){.read = read, .write = write, .lookup = lookup, .list = list};

static vfs_node_t node = (vfs_node_t){.name = "stdout",
                                      .size = 0,

                                      .ops = &node_ops};

vfs_node_t *stdout_new() { return &node; }
