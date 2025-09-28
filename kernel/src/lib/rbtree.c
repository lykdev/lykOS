#include "rbtree.h"

static void left_rotate(rbtree_t *r, rbtree_node_t *x)
{
    rbtree_node_t *y = x->right;

    x->right = y->left;
    if (y->left)
        y->left->parent = x;

    y->parent = x->parent;
    if (!x->parent)
        r->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}

static void right_rotate(rbtree_t *r, rbtree_node_t *x)
{
    rbtree_node_t *y = x->left;

    x->left = y->right;
    if (y->right)
        y->right->parent = x;

    y->parent = x->parent;
    if (!x->parent)
        r->root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;

    y->right = x;
    x->parent = y;
}

static void fix_insert(rbtree_t *r, rbtree_node_t *z)
{
    while (z->parent && z->parent->red)
    {
        rbtree_node_t *gp = z->parent->parent;
        if (!gp)
            break;

        if (z->parent == gp->left)
        {
            rbtree_node_t *y = gp->right;

            if (y && y->red)
            {
                z->parent->red = false;
                y->red = false;
                gp->red = true;
                z = gp;
            }
            else
            {
                if (z == z->parent->right)
                {
                    z = z->parent;
                    left_rotate(r, z);
                }
                z->parent->red = false;
                gp->red = true;
                right_rotate(r, gp);
            }
        }
        else
        {
            rbtree_node_t *y = gp->left;

            if (y && y->red)
            {
                z->parent->red = false;
                y->red = false;
                gp->red = true;
                z = gp;
            }
            else
            {
                if (z == z->parent->left)
                {
                    z = z->parent;
                    right_rotate(r, z);
                }
                z->parent->red = false;
                gp->red = true;
                left_rotate(r, gp);
            }
        }
    }

    if (r->root)
        r->root->red = false;
}

static rbtree_node_t *tree_min(rbtree_node_t *x)
{
    while (x && x->left)
        x = x->left;

    return x;
}

static void transplant(rbtree_t *r, rbtree_node_t *u, rbtree_node_t *v)
{
    if (!u->parent)
        r->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;

    if (v)
        v->parent = u->parent;
}

/*
 * Public functions.
 */

rbtree_node_t *rb_find(rbtree_t *r, u64 key)
{
    rbtree_node_t *n = r->root;

    while (n)
    {
        if (key == n->key)
            return n;

        if (key < n->key)
            n = n->left;
        else
            n = n->right;
    }

    return NULL;
}

bool rb_insert(rbtree_t *r, rbtree_node_t *z)
{
    rbtree_node_t *y = NULL;
    rbtree_node_t *x = r->root;

    while (x)
    {
        y = x;
        if (z->key == x->key)
            return false;

        if (z->key < x->key)
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;
    z->left = NULL;
    z->right = NULL;
    z->red = true;

    if (!y)
        r->root = z;
    else if (z->key < y->key)
        y->left = z;
    else
        y->right = z;

    fix_insert(r, z);
    return true;
}

rbtree_node_t *rb_remove(rbtree_t *r, u64 key)
{
    rbtree_node_t *z = rb_find(r, key);
    if (!z)
        return NULL;

    rbtree_node_t *y = z;
    bool y_original_red = y->red;
    rbtree_node_t *x = NULL;
    rbtree_node_t *x_parent = NULL;

    if (!z->left)
    {
        x = z->right;
        x_parent = z->parent;
        transplant(r, z, z->right);
    }
    else if (!z->right)
    {
        x = z->left;
        x_parent = z->parent;
        transplant(r, z, z->left);
    }
    else
    {
        y = tree_min(z->right);
        y_original_red = y->red;
        x = y->right;

        if (y->parent == z)
        {
            if (x)
                x->parent = y;
            x_parent = y;
        }
        else
        {
            transplant(r, y, y->right);
            y->right = z->right;
            if (y->right)
                y->right->parent = y;
            x_parent = y->parent;
        }

        transplant(r, z, y);
        y->left = z->left;
        if (y->left)
            y->left->parent = y;
        y->red = z->red;
    }

    if (!y_original_red)
    {
        rbtree_node_t *w;

        while ((x != r->root) && ((x == NULL) || (x->red == false)))
        {
            if (x_parent == NULL)
                break;

            if (x == x_parent->left)
            {
                w = x_parent->right;

                if (w && w->red)
                {
                    w->red = false;
                    x_parent->red = true;
                    left_rotate(r, x_parent);
                    w = x_parent->right;
                }

                if ((!w->left || !w->left->red) && (!w->right || !w->right->red))
                {
                    if (w)
                        w->red = true;
                    x = x_parent;
                    x_parent = x->parent;
                }
                else
                {
                    if (!w->right || !w->right->red)
                    {
                        if (w->left)
                            w->left->red = false;
                        w->red = true;
                        right_rotate(r, w);
                        w = x_parent->right;
                    }

                    if (w)
                        w->red = x_parent->red;
                    x_parent->red = false;
                    if (w && w->right)
                        w->right->red = false;
                    left_rotate(r, x_parent);
                    x = r->root;
                    break;
                }
            }
            else
            {
                w = x_parent->left;

                if (w && w->red)
                {
                    w->red = false;
                    x_parent->red = true;
                    right_rotate(r, x_parent);
                    w = x_parent->left;
                }

                if ((!w->left || !w->left->red) && (!w->right || !w->right->red))
                {
                    if (w)
                        w->red = true;
                    x = x_parent;
                    x_parent = x->parent;
                }
                else
                {
                    if (!w->left || !w->left->red)
                    {
                        if (w->right)
                            w->right->red = false;
                        w->red = true;
                        left_rotate(r, w);
                        w = x_parent->left;
                    }

                    if (w)
                        w->red = x_parent->red;
                    x_parent->red = false;
                    if (w && w->left)
                        w->left->red = false;
                    right_rotate(r, x_parent);
                    x = r->root;
                    break;
                }
            }
        }

        if (x)
            x->red = false;
    }

    z->parent = NULL;
    z->left = NULL;
    z->right = NULL;
    z->red = false;

    return z;
}

rbtree_node_t *rb_first(rbtree_t *r)
{
    return tree_min(r->root);
}

rbtree_node_t *rb_next(rbtree_node_t *n)
{
    if (!n)
        return NULL;

    if (n->right)
        return tree_min(n->right);

    rbtree_node_t *p = n->parent;
    while (p && n == p->right)
    {
        n = p;
        p = p->parent;
    }

    return p;
}
