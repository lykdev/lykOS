#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <lib/errno.h>
#include <sys/resource.h>

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

sys_ret_t syscall_close(int fd)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    if (proc->resource_table.resources[fd] == NULL)
        return (sys_ret_t) {0, EBADF};

    proc->resource_table.resources[fd] = NULL;
    return (sys_ret_t) {0, EOK};
}

sys_ret_t syscall_ioctl(int fd, int op, void *argp)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return (sys_ret_t) {0, EBADF};
    vnode_t *node = res->node;
    if (node == NULL)
        return (sys_ret_t) {0, EBADF};

    return (sys_ret_t) {
        0,
        node->ops->ioctl(node, op, argp)
    };
}

sys_ret_t syscall_open(const char *path, int flags, int mode)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    vnode_t *node;
    int ret = vfs_open(path, &node);
    if (ret != 0)
        return (sys_ret_t) {0, ret};

    return (sys_ret_t) {
        resource_create(&proc->resource_table, node, 0, RESOURCE_READ | RESOURCE_WRITE),
        EOK
    };
}

sys_ret_t syscall_read(int fd, void *buf, u64 count)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL || (res->flags & RESOURCE_READ) == 0)
        return (sys_ret_t) {0, EBADF};
    vnode_t *node = res->node;
    if (node == NULL)
        return (sys_ret_t) {0, EBADF};

    u64 out;
    int err = node->ops->read(node, res->offset, buf, count, &out);

    return (sys_ret_t) {out, err};
}

sys_ret_t syscall_seek(int fd, u64 offset, int whence)
{
    // TODO: replace this with an actual solution
    if (fd == 0 | fd == 1 || fd == 2) // stdout & stderr
        return (sys_ret_t) {offset, EOK};

    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, fd);
    if (res == NULL)
        return (sys_ret_t) {0, EBADF};
    vnode_t *node = res->node;
    if (node == NULL)
        return (sys_ret_t) {0, EBADF};

    u64 new_offset;
    switch (whence)
    {
    case SEEK_SET: new_offset = offset; break;
    case SEEK_CUR: new_offset = res->offset + offset; break;
    case SEEK_END: new_offset = res->node->size + offset; break;
    default:       return (sys_ret_t) {0, EINVAL};
    }

    if (new_offset > res->node->size)
        return (sys_ret_t) {0, EINVAL};

    res->offset = new_offset;
    return (sys_ret_t) {new_offset, EOK};
}

sys_ret_t syscall_write(int fd, void *buf, u64 count)
{
    // TODO: replace this with an actual solution
    if (fd <= 2) // stdout & stderr
    {
        if (fd == 1 || fd == 2)
            log("%s", buf);
        return (sys_ret_t) {count, EOK};
    }

    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res  = resource_get(&proc->resource_table, fd);
    if (res == NULL || (res->flags & RESOURCE_WRITE) == 0)
        return (sys_ret_t) {0, EBADF};
    vnode_t *node = res->node;
    if (node == NULL)
        return (sys_ret_t) {0, EBADF};

    u64 out;
    int err = node->ops->write(node, res->offset, buf, count, &out);

    return (sys_ret_t) {out, err};
}
