#pragma once

#include <fs/vfs.h>

typedef struct
{
    uint16_t family;
    uint16_t port;
    uint64_t addr;
}
socket_addr_t;

int socket_create(vfs_node_t *handle, int domain, int type, int protocol);
int socket_bind(vfs_node_t *handle, const socket_addr_t *addr, int addrlen);
int socket_listen(vfs_node_t *handle, int backlog);
int socket_accept(vfs_node_t *handle, vfs_node_t **new_node, socket_addr_t *addr, int *addrlen);
int socket_connect(vfs_node_t *handle, const socket_addr_t *addr, int addrlen);
int socket_send(vfs_node_t *handle, const void *buf, size_t len, int flags);
int socket_recv(vfs_node_t *handle, void *buf, size_t len, int flags);
int socket_close(vfs_node_t *handle);
