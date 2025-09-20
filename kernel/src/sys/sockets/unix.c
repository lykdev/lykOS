#include "socket.h"

#include <common/log.h>
#include <lib/errno.h>
#include <lib/string.h>
#include <mm/heap.h>

#define AF_UNIX 1

typedef struct
{
    socket_t socket;
    vnode_t *bound_vnode;
}
socket_unix_t;

static socket_t *create_socket(int type, int protocol);

static socket_family_t family = {
    .family = AF_UNIX,
    .create_socket = create_socket
};

static int bind(socket_t *self, const char *addr)
{
    spinlock_acquire(&self->lock);

    if (self->state >= SOCKET_STATE_BOUND)
    {
        spinlock_release(&self->lock);
        return -EINVAL;
    }

    socket_unix_t *usock = (socket_unix_t *)self;

    vnode_t *addr_node;
    if (vfs_create(addr, VNODE_SOCKET, &addr_node) < 0)
    {
        spinlock_release(&self->lock);
        return -EADDRINUSE;
    }

    // Link the vfs node and socket.
    addr_node->mp_data = usock;
    usock->bound_vnode = addr_node;

    self->state = SOCKET_STATE_BOUND;

    spinlock_release(&self->lock);
    return EOK;
}

static int connect(socket_t *self, const char *addr)
{
    // TODO: theres a double lock issue here
    spinlock_acquire(&self->lock);

    vnode_t *target_node;
    vfs_open(addr, &target_node);
    if (!target_node || target_node->type != VNODE_SOCKET)
    {
        spinlock_release(&self->lock);
        return -ENOENT;
    }

    socket_unix_t *target_usock = (socket_unix_t *)target_node->mp_data;

    spinlock_acquire(&target_usock->socket.lock);

    if (target_usock->socket.state != SOCKET_STATE_LISTENING)
    {
        spinlock_release(&target_usock->socket.lock);
        spinlock_release(&self->lock);
        return -ECONNREFUSED;
    }

    self->state = SOCKET_STATE_CONNECTING;
    list_append(&target_usock->socket.accept_queue, &self->accept_queue_node);

    spinlock_release(&target_usock->socket.lock);
    spinlock_release(&self->lock);
    return EOK;
}

static int listen(socket_t *self)
{
    spinlock_acquire(&self->lock);

    self->state = SOCKET_STATE_LISTENING;

    spinlock_release(&self->lock);

    return EOK;
}

static int accept(socket_t *self, int flags, socket_t **new_sock)
{
    // TODO: use mutex
    while (true)
    {
        spinlock_acquire(&self->lock);

        if (!list_is_empty(&self->accept_queue))
        {
            socket_t *peer = LIST_GET_CONTAINER(list_pop_head(&self->accept_queue), socket_t, accept_queue_node);

            // Allocate a new socket.
            socket_unix_t *new_usock = (socket_unix_t *)create_socket(self->type, self->protocol);
            new_usock->socket.state = SOCKET_STATE_CONNECTED;
            new_usock->socket.peer = peer;

            peer->state = SOCKET_STATE_CONNECTED;
            peer->peer = &new_usock->socket;

            *new_sock = &new_usock->socket;
            spinlock_release(&self->lock);
            return EOK;
        }

        spinlock_release(&self->lock);
    }
}

static int send(socket_t *self, const void *buf, size_t len, int flags)
{
    return 0;
}

static int recv(socket_t *self, void *buf, size_t len, int flags)
{
    return 0;
}

static int close(socket_t *self)
{
    return 0;
}

static int shutdown(socket_t *self, int how)
{
    return 0;
}

static int getsockname(socket_t *self, char *addr)
{
    return 0;
}

static int getpeername(socket_t *self, char *addr)
{
    return 0;
}

static socket_ops_t ops = {
    .bind = bind,
    .connect = connect,
    .listen = listen,
    .accept = accept,
    .send = send,
    .recv = recv,
    .close = close,
    .shutdown = shutdown,
    .getsockname = getsockname,
    .getpeername = getpeername
};

static socket_t *create_socket(int type, int protocol)
{
    socket_unix_t *sock = heap_alloc(sizeof(socket_unix_t));
    if (!sock)
        return NULL;

    sock->socket = (socket_t) {
        .family = AF_UNIX,
        .type = type,
        .protocol = protocol,
        .state = SOCKET_STATE_NEW,
        .peer = NULL,
        .accept_queue = LIST_INIT,
        .ops = &ops,
        .lock = SPINLOCK_INIT
    };
    sock->bound_vnode = NULL;

    return &sock->socket;
}

void afunix_init()
{
    log("afunix: registering AF_UNIX socket family");

    socket_register_family(&family);
}
