#pragma once

#include <lib/def.h>

typedef struct rbtree_node
{
    u64 key;
    void *value;
    struct rbtree_node *parent;
    struct rbtree_node *left;
    struct rbtree_node *right;
    bool red;
}
rbtree_node_t;

typedef struct
{
    rbtree_node_t *root;
}
rbtree_t;

/// @param NODE Red-black tree node.
/// @param TYPE The type of the container.
/// @param MEMBER The rbtree_node_t member inside of the container.
/// @return Pointer to the container.
#define RBTREE_GET_CONTAINER(NODE, TYPE, MEMBER) ((TYPE *)((uptr)(NODE) - __builtin_offsetof(TYPE, MEMBER)))

#define RBTREE_INIT \
    (rb_tree_t) {r->root = NULL}

#define RBTREE_NODE_INIT \
    (rb_node_t) { .key = 0, .value = NULL, .parent = NULL, .left = NULL, .right = NULL, .red = false }

rbtree_node_t *rb_find(rbtree_t *r, u64 key);
bool rb_insert(rbtree_t *r, rbtree_node_t *node);
rbtree_node_t *rb_remove(rbtree_t *r, u64 key);
rbtree_node_t *rb_first(rbtree_t *r);
rbtree_node_t *rb_next(rbtree_node_t *n);
