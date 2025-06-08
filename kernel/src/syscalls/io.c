#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <sys/resource.h>

int syscall_close(int fd)
{
    proc_t *proc = syscall_get_proc();

    proc->resource_table.resources[fd] = NULL;

    return 0;
}

int syscall_open(const char *path, int flags, int mode)
{
    proc_t *proc = syscall_get_proc();

    vfs_node_t *node = vfs_lookup(path);
    if(node == NULL)
        return -1;

    return resource_create(&proc->resource_table, node, 0, RESOURCE_READ | RESOURCE_WRITE);
}

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
        return node->file_ops->write(node, res->offset, buf, count);
    else if (node->type == VFS_NODE_CHAR)
        return node->char_ops->write(node, res->offset, buf, count);
    else
        return -1;
}
