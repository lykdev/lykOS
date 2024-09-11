#include "list.h"

//#include <lib/assert.h>

bool list_is_empty(list_t *list)
{
    // Sanity check: make sure that the two statements are either both true or both false.
    //ASSERT (!((list->head == NULL) ^ (list->length == 0)));

    return list->head == NULL;
}

void list_append(list_t *list, list_node_t *node)
{
    if (list->tail != NULL) 
        list->tail->next = node;

    // Set appended node info.
    node->prev = list->tail;
    node->next = NULL;

    // Update head and tail.
    list->tail = node;
    if (list->head == NULL)
        list->head = list->tail;

    list->length++;
}

void list_prepend(list_t *list, list_node_t *node)
{
    if (list->head != NULL)
        list->head->prev = node;

    // Set prepended node info.
    node->prev = NULL;
    node->next = list->head;

    // Update head and tail.
    list->head = node;
    if (list->tail == NULL)
        list->tail = list->head;

    list->length++;
}

void list_remove(list_t *list, list_node_t *node)
{
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;

    if (list->head == node)
        list->head = node->next;
    if (list->tail == node)
        list->tail = node->prev;

    list->length--;
}
