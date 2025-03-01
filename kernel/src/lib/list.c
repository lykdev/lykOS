#include "list.h"

#include <common/assert.h>

bool list_is_empty(list_t *list)
{
  // Sanity check: make sure that the two statements are either both true or
  // both false.
  ASSERT(!((list->head == NULL) ^ (list->length == 0)));

  return list->head == NULL;
}

void list_insert_after(list_t *list, list_node_t *pos, list_node_t *new)
{
  if (pos == NULL) // This case works only if the list is empty.
  {
    ASSERT(list_is_empty(list));

    list->head = new;
    list->tail = new;
    new->prev = NULL;
    new->next = NULL;
  } else
  {
    // Middle/new node.
    new->prev = pos;
    new->next = pos->next;

    // Right node.
    if (new->next != NULL)
      new->next->prev = new;

    // Left node.
    pos->next = new;

    // Update tail if needed.
    if (list->tail == pos)
      list->tail = new;
  }

  list->length++;
}

void list_insert_before(list_t *list, list_node_t *pos, list_node_t *new)
{
  if (pos == NULL) // This case works only if the list is empty.
  {
    ASSERT(list_is_empty(list));

    list->head = new;
    list->tail = new;
    new->prev = NULL;
    new->next = NULL;
  } else
  {
    // Middle/new node.
    new->prev = pos->prev;
    new->next = pos;

    // Left node
    if (new->prev != NULL)
      new->prev->next = new;

    // Right node.
    pos->prev = new;

    // Update head if needed.
    if (list->head == pos)
      list->head = new;
  }

  list->length++;
}

void list_append(list_t *list, list_node_t *node)
{
  list_insert_after(list, list->tail, node);
}

void list_prepend(list_t *list, list_node_t *node)
{
  list_insert_before(list, list->head, node);
}

void list_remove(list_t *list, list_node_t *node)
{
  ASSERT(node != NULL);

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

list_node_t *list_pop_head(list_t *list)
{
  list_node_t *node = list->head;
  if (node != NULL)
    list_remove(list, list->head);

  return node;
}

list_node_t *list_pop_tail(list_t *list)
{
  list_node_t *node = list->tail;
  if (node != NULL)
    list_remove(list, list->tail);

  return node;
}
