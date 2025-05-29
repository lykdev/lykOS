#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>

int syscall_write(int fd, void *buf, u64 count)
{
    // TODO: remove this from here
    if (fd == 1)
        log("%s", buf);

    proc_t *proc = syscall_get_proc();

    resource_t *res  = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -1;

    vfs_node_t *node = res->node;
    if (node == NULL)
        return -1;

    if (node->type == VFS_NODE_FILE)
        node->file_ops->write(node, res->offset, buf, count);
    else if (node->type == VFS_NODE_CHAR)
        node->char_ops->write(node, res->offset, buf, count); 
    else
        return -1;
}
