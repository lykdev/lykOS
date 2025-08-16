#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <lib/errno.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <sys/sockets/socket.h>

i64 syscall_accept(int sockfd, const char *addr)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VFS_NODE_SOCKET)
        return -EBADF;

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return -EINVAL;

    // Call the internal accept logic for the socket.
    socket_t *new_socket;
    i64 ret = socket->ops->accept(socket, 0, &new_socket);
    if (ret < 0)
        return ret;

    vfs_node_t *handle = heap_alloc(sizeof(vfs_node_t));
    if (!handle)
        return -ENOMEM; // TODO: destroy the socket
    handle->type = VFS_NODE_SOCKET;
    handle->mp_data = new_socket;

    int fd = resource_create(&proc->resource_table,
                             handle,
                             0,
                             RESOURCE_READ | RESOURCE_WRITE);
    if (fd < 0)
    {
        //TODO: destroy the socket
        heap_free(handle);
        return -EMFILE;
    }

    return fd;
}

i64 syscall_bind(int sockfd, const char *addr)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VFS_NODE_SOCKET)
        return -EBADF;

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return -EINVAL;

    // Call the internal bind logic for the socket.
    return socket->ops->bind(socket, addr);
}

i64 syscall_connect(int sockfd, const char *addr)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VFS_NODE_SOCKET)
        return -EBADF;

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return -EINVAL;

    // Call the internal connect logic for the socket.
    return socket->ops->connect(socket, addr);
}

i64 syscall_getpeername(int sockfd, char *addr)
{

}

i64 syscall_getsockname(int sockfd, char *addr)
{

}

i64 syscall_listen(int sockfd, int backlog)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VFS_NODE_SOCKET)
        return -EBADF;

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return -EINVAL;

    // Call the internal listen logic for the socket.
    return socket->ops->listen(socket);
}

i64 syscall_recv(int sockfd, void *buf, size_t len, int flags)
{

}

i64 syscall_send(int sockfd, const void *buf, size_t len, int flags)
{

}

i64 syscall_shutdown(int sockfd, int how)
{

}

i64 syscall_socket(int domain, int type, int protocol)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    socket_family_t *family = socket_get_family(domain);
    if (!family)
        return -EINVAL;

    vfs_node_t *handle = heap_alloc(sizeof(vfs_node_t));
    if (!handle)
        return -ENOMEM;

    socket_t *socket = family->create_socket(type, protocol);
    if (!socket)
    {
        heap_free(handle);
        return -ENOMEM;
    }
    handle->type = VFS_NODE_SOCKET;
    handle->mp_data = socket;

    int fd = resource_create(&proc->resource_table,
                             handle,
                             0,
                             RESOURCE_READ | RESOURCE_WRITE);
    if (fd < 0)
    {
        //TODO: destroy the socket
        heap_free(handle);
        return -EMFILE;
    }

    return fd;
}
