#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <sys/resource.h>

int syscall_read(int fd, void *buf, u64 count)
{
    proc_t *proc = syscall_get_proc();

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -1;

    vfs_node_t *node = resource_get(&proc->resource_table, fd)->node;
    if (node == NULL)
        return -1;

    return node->file_ops->read(node, res->offset, buf, count);
}
