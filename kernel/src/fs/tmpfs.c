#include "tmpfs.h"

#include <fs/vfs.h>
#include <common/assert.h>
#include <common/hhdm.h>
#include <common/sync/spinlock.h>
#include <lib/def.h>
#include <lib/errno.h>
#include <lib/list.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/pmm.h>
#include <mm/heap.h>

typedef struct
{
    vnode_t vn;

    list_t children;
    list_t pages;

    list_node_t list_node;
}
tmpfs_node_t;

typedef struct
{
    void *data;

    list_node_t list_node;
}
tmpfs_page_t;

static int tmpfs_open  (vnode_t *self, const char *name, vnode_t **out);
static int tmpfs_close (vnode_t *self);
static int tmpfs_read  (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
static int tmpfs_write (vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out);
static int tmpfs_list  (vnode_t *self, u64 *hint, const char **out);
static int tmpfs_create(vnode_t *self, char *name, vnode_type_t t, vnode_t **out);
static int tmpfs_ioctl (vnode_t *self, u64 request, void *args);

static vnode_ops_t tmpfs_ops = {
    .open   = tmpfs_open,
    .close  = tmpfs_close,
    .read   = tmpfs_read,
    .write  = tmpfs_write,
    .list   = tmpfs_list,
    .create = tmpfs_create,
    .ioctl  = tmpfs_ioctl,
};

static int tmpfs_open(vnode_t *self, const char *name, vnode_t **out)
{
    ASSERT(name && out);

    if (self->type != VNODE_DIR)
    {
        *out = NULL;
        return ENOTDIR;
    }

    tmpfs_node_t *parent = (tmpfs_node_t *)self;
    FOREACH(n, parent->children)
    {
        tmpfs_node_t *child = LIST_GET_CONTAINER(n, tmpfs_node_t, list_node);
        if (strcmp(child->vn.name, name) == 0)
        {
            VN_HOLD(&child->vn);
            *out = &child->vn;
            return EOK;
        }
    }

    *out = NULL;
    return ENOENT;
}

static int tmpfs_close(vnode_t *self)
{
    VN_RELE(self);

    return EOK;
}

static int tmpfs_read(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{
    ASSERT(buffer && out);

    tmpfs_node_t *node = (tmpfs_node_t *)self;

    u64 page_start = 0;
    u64 copied = 0;
    FOREACH(n, node->pages)
    {
        tmpfs_page_t *page = LIST_GET_CONTAINER(n, tmpfs_page_t, list_node);

        if (offset >= page_start && page_start < offset + count)
        {
            u64 page_offset = offset - page_start;
            u64 bytes_to_copy = MIN(ARCH_PAGE_GRAN - page_offset, count - copied);

            memcpy(
                (u8 *)buffer + copied,
                (u8 *)page->data + page_offset,
                bytes_to_copy
            );
            copied += bytes_to_copy;
        }

        page_start += ARCH_PAGE_GRAN;
    }

    *out = copied;
    return EOK;
}

static int tmpfs_write(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{
    ASSERT(buffer && out);

    tmpfs_node_t *node = (tmpfs_node_t *)self;

    u64 needed_page_count = CEIL(offset + count, ARCH_PAGE_GRAN) / ARCH_PAGE_GRAN;
    u64 file_page_count = CEIL(node->vn.size, ARCH_PAGE_GRAN) / ARCH_PAGE_GRAN;
    if (needed_page_count > file_page_count)
        for (u64 i = 0; i < needed_page_count - file_page_count; i++)
        {
            tmpfs_page_t *page = heap_alloc(sizeof(tmpfs_page_t));
            *page = (tmpfs_page_t) {
                .data = (void*)((uptr)pmm_alloc(0) + HHDM),
                .list_node = LIST_NODE_INIT
            };
            list_append(&node->pages, &page->list_node);
        }

    u64 page_start = 0;
    u64 copied = 0;
    FOREACH(n, node->pages)
    {
        tmpfs_page_t *page = LIST_GET_CONTAINER(n, tmpfs_page_t, list_node);

        if (offset >= page_start && page_start < offset + count)
        {
            u64 page_offset = offset - page_start;
            u64 bytes_to_copy = MIN(ARCH_PAGE_GRAN - page_offset, count - copied);

            memcpy(
                (u8 *)page->data + page_offset,
                (u8 *)buffer + copied,
                bytes_to_copy
            );

            copied += bytes_to_copy;
        }

        page_start += ARCH_PAGE_GRAN;
    }

    // Update vnode size if needed.
    if (offset + copied > node->vn.size)
        node->vn.size = offset + copied;

    *out = copied;
    return EOK;
}

static int tmpfs_list(vnode_t *self, u64 *hint, const char **out)
{
    ASSERT(hint && out);
}

static int tmpfs_create(vnode_t *self, char *name, vnode_type_t t, vnode_t **out)
{
    ASSERT(name && out);

    tmpfs_node_t *parent = (tmpfs_node_t *)self;
    tmpfs_node_t *child  = heap_alloc(sizeof(tmpfs_node_t));

    *child = (tmpfs_node_t) {
        .vn = (vnode_t) {
            .type = t,
            .size = 0,
            .ops  = &tmpfs_ops,
            .slock = SPINLOCK_INIT,
            .ref_count = 1
        },
        .children = LIST_INIT,
        .pages = LIST_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy(child->vn.name, name);

    list_append(&parent->children, &child->list_node);

    *out = &child->vn;
    return EOK;
}

static int tmpfs_ioctl(vnode_t *self, u64 request, void *args)
{
    return ENOTSUP;
}

void tmpfs_init()
{
    vnode_t *root = heap_alloc(sizeof(vnode_t));
    *root = (vnode_t) {
        .type = VNODE_DIR,
        .size = 0,
        .ops  = &tmpfs_ops,
        .slock = SPINLOCK_INIT,
        .ref_count = 1
    };
    strcpy(root->name, "tmp");

    vfs_mountpoint_t *mp = heap_alloc(sizeof(vfs_mountpoint_t));
    mp->root_node = root;

    vfs_mount("/tmp", mp);
}
