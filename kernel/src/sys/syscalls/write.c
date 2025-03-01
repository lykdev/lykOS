#include "syscall.h"

#include <fs/vfs.h>

#include <common/log.h>

int syscall_write(int fd, u64 count, void *buf)
{
    log("WRITE");
    proc_t *proc = syscall_get_proc();
    
    vfs_node_t *node = resource_get(&proc->resource_table, fd)->node;
    if (node == NULL)
        return -1;

    node->ops->write(node, 0, count, buf);
    return 0;
}
