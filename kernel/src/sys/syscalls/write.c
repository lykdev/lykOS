#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>

int syscall_write(int fd, void *buf, u64 count)
{
    log("WRITE");
    proc_t *proc = syscall_get_proc();

    resource_t *res  = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -1;
        
    vfs_node_t *node = res->node;
    if (node == NULL)
        return -1;

    node->ops->write(node, 0, count, buf);

    return count;
}
