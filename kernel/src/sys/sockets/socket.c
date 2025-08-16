#include "socket.h"

list_t g_socket_family_list = LIST_INIT;

bool socket_register_family(socket_family_t *family)
{
    // If the family is already registered return false.
    FOREACH (n, g_socket_family_list)
    {
        socket_family_t *f = LIST_GET_CONTAINER(n, socket_family_t, socket_family_list_node);
        if (f->family == family->family)
            return false;
    }

    list_append(&g_socket_family_list, &family->socket_family_list_node);

    return true;
}

socket_family_t *socket_get_family(int family)
{
    FOREACH (n, g_socket_family_list)
    {
        socket_family_t *f = LIST_GET_CONTAINER(n, socket_family_t, socket_family_list_node);
        if (f->family == family)
            return f;
    }

    return NULL;
}
