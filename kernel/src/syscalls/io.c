#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <lib/errno.h>
#include <sys/resource.h>

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

i64 syscall_close(int fd)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    if (proc->resource_table.resources[fd] == NULL)
        return -EBADF;

    proc->resource_table.resources[fd] = NULL;
    return EOK;
}

i64 syscall_ioctl(int fd, int op, void *argp)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -EBADF;
    vnode_t *node = res->node;
    if (node == NULL)
        return -EBADF;

    return node->ops->ioctl(node, op, argp);
}

i64 syscall_open(const char *path, int flags, int mode)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    vnode_t *node;
    int ret = vfs_open(path, &node);
    if(ret < 0)
        return ret;

    return resource_create(&proc->resource_table, node, 0, RESOURCE_READ | RESOURCE_WRITE);
}

i64 syscall_read(int fd, void *buf, u64 count)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL || (res->flags & RESOURCE_READ) == 0)
        return -EBADF;
    vnode_t *node = res->node;
    if (node == NULL)
        return -EBADF;

    u64 out;
    int ret = node->ops->read(node, res->offset, buf, count, &out);

    if (ret < 0)
        return ret;
    else
        return out;
}

i64 syscall_seek(int fd, u64 offset, int whence)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return -EBADF;
    vnode_t *node = res->node;
    if (node == NULL)
        return -EBADF;

    switch (whence)
    {
    case SEEK_SET: return offset;
    case SEEK_CUR: return res->offset + offset;
    case SEEK_END: return res->node->size + offset;
    default:       return -EINVAL;
    }
}

i64 syscall_write(int fd, void *buf, u64 count)
{
    // TODO: replace this with an actual solution
    if (fd == 1 || fd == 2) // stdout & stderr
    {
        log("%s", buf);
        return count;
    }

    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res  = resource_get(&proc->resource_table, fd);
    if (res == NULL || (res->flags & RESOURCE_WRITE) == 0)
        return -EBADF;
    vnode_t *node = res->node;
    if (node == NULL)
        return -EBADF;

    u64 out;
    int ret = node->ops->write(node, res->offset, buf, count, &out);

    if (ret < 0)
        return ret;
    else
        return out;
}
