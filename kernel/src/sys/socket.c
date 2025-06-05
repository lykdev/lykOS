#include "socket.h"

#include <mm/heap.h>

typedef struct
{
    int domain;
    int type;
    int protocol;
    int state; // e.g. CLOSED, LISTENING, CONNECTED
    // ... buffer pointers, queues, etc.
}
socket_internal_t;

int socket_socket(vfs_node_t *handle, int domain, int type, int protocol)
{
    if (!handle)
        return -1;
    socket_internal_t *sock = heap_alloc(sizeof(socket_internal_t));
    if (!sock)
        return -1;

    sock->domain = domain;
    sock->type = type;
    sock->protocol = protocol;
    sock->state = 0; // CLOSED

    handle->mp_data = sock;
    return 0;
}

int socket_bind(struct vfs_node *handle, const socket_addr_t *addr, int addrlen)
{
    if (!handle || !handle->mp_data) return -1;
    socket_internal_t *sock = handle->mp_data;

    // store binding info inside socket_internal or handle it
    // dummy example: just mark bound state
    sock->state = 1; // BOUND
    return 0;
}

int socket_listen(struct vfs_node *node, int backlog)
{
    if (!node || !node->mp_data)
        return -1;
    socket_internal_t *sock = node->mp_data;

    if (sock->state != 1)
        return -1; // must be bound first
    sock->state = 2; // LISTENING
    return 0;
}

int socket_accept(struct vfs_node *handle, struct vfs_node **new_node, socket_addr_t *addr, int *addrlen)
{
    if (!handle || !handle->mp_data || !new_node)
        return -1;
    socket_internal_t *sock = handle->mp_data;

    if (sock->state != 2)
        return -1; // must be listening

    // Allocate new vfs_node for the accepted socket
    struct vfs_node *client_node = vfs_node_create_socket();
    if (!client_node)
        return -1;

    // Initialize client socket internals
    socket_internal_t *client_sock = heap_alloc(sizeof(socket_internal_t));
    if (!client_sock)
    {
        vfs_node_put(client_node);
        return -1;
    }

    *client_node->socket_ops = *handle->socket_ops; // copy ops ptrs
    client_node->mp_data = client_sock;
    client_sock->state = 3; // CONNECTED

    *new_node = client_node;
    if (addr && addrlen) {
        // fill client addr info if available
        // dummy zero for now
        *addrlen = 0;
    }

    return 0;
}

int socket_connect(struct vfs_node *handle, const socket_addr_t *addr, int addrlen)
{
    if (!handle || !handle->mp_data)
        return -1;
    socket_internal_t *sock = handle->mp_data;

    sock->state = 3; // CONNECTED
    return 0;
}

int socket_send(struct vfs_node *handle, const void *buf, size_t len, int flags)
{
    if (!handle || !handle->mp_data)
        return -1;

    // TODO: push data into send queue or transmit buffer
    // Just a dummy: pretend all sent immediately
    return (int)len;
}

int socket_recv(struct vfs_node *handle, void *buf, size_t len, int flags)
{
    if (!handle || !handle->mp_data)
        return -1;

    // TODO: read data from recv queue or buffer
    // Dummy: no data available
    return 0;
}

int socket_close(struct vfs_node *handle)
{
    if (!handle || !handle->mp_data)
        return -1;

    socket_internal_t *sock = handle->mp_data;
    heap_free(sock);
    handle->mp_data = NULL;
    return 0;
}
