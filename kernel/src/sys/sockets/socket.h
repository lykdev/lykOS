#pragma once

#include <fs/vfs.h>
#include <lib/list.h>

#define SOCK_STREAM    1
#define SOCK_DGRAM     2

#define SOCK_NONBLOCK  04000

typedef struct socket_ops socket_ops_t;
typedef struct socket socket_t;

typedef struct
{
    int family;
    socket_t *(*create_socket)(int type, int protocol);

    list_node_t socket_family_list_node;
}
socket_family_t;

struct socket_ops
{
    int (*bind)(socket_t *self, const char *addr);
    int (*connect)(socket_t *self, const char *addr);
    int (*listen)(socket_t *self);
    int (*accept)(socket_t *self, int flags, socket_t **new_sock);
    int (*send)(socket_t *self, const void *buf, size_t len, int flags);
    int (*recv)(socket_t *self, void *buf, size_t len, int flags);
    int (*close)(socket_t *self);
    int (*shutdown)(socket_t *self, int how);
    int (*getsockname)(socket_t *self, char *addr);
    int (*getpeername)(socket_t *self, char *addr);
};

typedef enum
{
    SOCKET_STATE_NEW,
    SOCKET_STATE_BOUND,
    SOCKET_STATE_LISTENING,
    SOCKET_STATE_CONNECTING,
    SOCKET_STATE_CONNECTED,
    SOCKET_STATE_CLOSED
}
socket_state_t;

struct socket
{
    int family, type, protocol;
    socket_state_t state;

    socket_t *peer;
    list_t accept_queue;

    socket_ops_t *ops;
    void *private_data;

    list_node_t accept_queue_node;
    spinlock_t lock;
};

extern list_t g_socket_family_list;

bool socket_register_family(socket_family_t *family);

socket_family_t *socket_get_family(int family);
