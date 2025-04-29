#include "syscall.h"

#include <common/log.h>

int syscall_lseek(int fd, u64 offset, int whence)
{
    log("LSEEK");
    proc_t *proc = syscall_get_proc();

    resource_t *res  = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -1;

    vfs_node_t *node = res->node;
    if (node == NULL)
        return -1;

    switch (whence)
    {
    default:
        return -1;
        break;
    }

    return offset;
}
