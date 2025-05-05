#include "syscall.h"

#include <common/log.h>

#define SEEK_SET  0
#define SEEK_CURR 1
#define SEEK_END  2

int syscall_seek(int fd, u64 offset, int whence)
{
    proc_t *proc = syscall_get_proc();

    resource_t *res  = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -1;
    vfs_node_t *node = res->node;
    if (node == NULL)
        return -1;

    switch (whence)
    {
    case SEEK_SET:
        res->offset = offset;
        break;
    case SEEK_CURR:
        res->offset += offset;
        break;
    case SEEK_END:
        res->offset = res->node->size + offset;
    default:
        return -1;
    }

    return res->offset;
}
