#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>
#include <lib/errno.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <sys/sockets/socket.h>

sys_ret_t syscall_accept(int sockfd, const char *addr)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VNODE_SOCKET)
        return (sys_ret_t) {0, EBADF};

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return (sys_ret_t) {0, EINVAL};

    // Call the internal accept logic for the socket.
    socket_t *new_socket;
    i64 ret = socket->ops->accept(socket, 0, &new_socket);
    if (ret != 0)
        return (sys_ret_t) {0, ret};

    vnode_t *handle = heap_alloc(sizeof(vnode_t));
    if (!handle)
        return (sys_ret_t) {0, ENOMEM}; // TODO: destroy the socket
    handle->type = VNODE_SOCKET;
    handle->mp_data = new_socket;

    int fd = resource_create(&proc->resource_table,
                             handle,
                             0,
                             RESOURCE_READ | RESOURCE_WRITE);
    if (fd < 0)
    {
        //TODO: destroy the socket
        heap_free(handle);
        return (sys_ret_t) {0, EMFILE};
    }

    return (sys_ret_t) {fd, EOK};
}

sys_ret_t syscall_bind(int sockfd, const char *addr)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VNODE_SOCKET)
        return (sys_ret_t) {0, EBADF};

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return (sys_ret_t) {0, EINVAL};

    // Call the internal bind logic for the socket.
    return (sys_ret_t) {0, socket->ops->bind(socket, addr)};
}

sys_ret_t syscall_connect(int sockfd, const char *addr)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VNODE_SOCKET)
        return (sys_ret_t) {0, EBADF};

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return (sys_ret_t) {0, EINVAL};

    // Call the internal connect logic for the socket.
    return (sys_ret_t) {0, socket->ops->connect(socket, addr)};
}

sys_ret_t syscall_getpeername(int sockfd, char *addr)
{

}

sys_ret_t syscall_getsockname(int sockfd, char *addr)
{

}

sys_ret_t syscall_listen(int sockfd, int backlog)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    resource_t *res = resource_get(&proc->resource_table, sockfd);
    if (!res || !res->node || res->node->type != VNODE_SOCKET)
        return (sys_ret_t) {0, EBADF};

    socket_t *socket = (socket_t *)res->node->mp_data;
    if (!socket)
        return (sys_ret_t) {0, EINVAL};

    // Call the internal listen logic for the socket.
    return (sys_ret_t) {0, socket->ops->listen(socket)};
}

sys_ret_t syscall_recv(int sockfd, void *buf, size_t len, int flags)
{

}

sys_ret_t syscall_send(int sockfd, const void *buf, size_t len, int flags)
{

}

sys_ret_t syscall_shutdown(int sockfd, int how)
{

}

sys_ret_t syscall_socket(int domain, int type, int protocol)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;

    socket_family_t *family = socket_get_family(domain);
    if (!family)
        return (sys_ret_t) {0, EINVAL};

    vnode_t *handle = heap_alloc(sizeof(vnode_t));
    if (!handle)
        return (sys_ret_t) {0, ENOMEM};

    socket_t *socket = family->create_socket(type, protocol);
    if (!socket)
    {
        heap_free(handle);
        return (sys_ret_t) {0, ENOMEM};
    }
    handle->type = VNODE_SOCKET;
    handle->mp_data = socket;

    int fd = resource_create(&proc->resource_table,
                             handle,
                             0,
                             RESOURCE_READ | RESOURCE_WRITE);
    if (fd < 0)
    {
        //TODO: destroy the socket
        heap_free(handle);
        return (sys_ret_t) {0, EMFILE};
    }

    return (sys_ret_t) {fd, EOK};
}
