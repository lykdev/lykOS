#include <common/log.h>
#include <sys/streams/streams.h>

static u64 read(vfs_node_t *self, u64 offset, void *buffer, u64 count)
{
    return 0;
}

static u64 write(vfs_node_t *self, u64 offset, void *buffer, u64 count)
{
    log("%s", buffer);
    return count;
}

static vfs_node_ops_t node_ops = (vfs_node_ops_t) {
    .read = read,
    .write = write,
    .lookup = NULL,
    .list = NULL
};

static vfs_node_t node = (vfs_node_t) {
    .name = "stdout",
    .size = 0,
    .ops = &node_ops
};

vfs_node_t *stdout_new()
{
    return &node;
}
