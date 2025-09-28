#include "vfs.h"
#include "arch/types.h"
#include "lib/rbtree.h"

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <common/panic.h>
#include <lib/path.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>

static list_t g_fs_type_list = LIST_INIT;
static spinlock_t g_fs_type_list_slock = SPINLOCK_INIT;

/*
 * Mount points
 */

typedef struct trie_node_t trie_node_t;

struct trie_node_t
{
    /// @brief The path component separated by delimiting characters.
    char comp[32];
    /// @brief Pointer to the mountpoint, NULL if no mount here.
    vnode_t *root_vn;
    trie_node_t *children[16];
    uint children_cnt;
};
static trie_node_t g_trie_root;

static trie_node_t *find_child(trie_node_t *parent, const char *comp, u64 length)
{
    for (uint i = 0; i < parent->children_cnt; i++)
        if (strncmp(parent->children[i]->comp, comp, length) == 0)
            return parent->children[i];
    return NULL;
}

static char *vfs_get_mountpoint(const char *path, vnode_t **out)
{
    trie_node_t *current = &g_trie_root;

    while (*path)
    {
        while(*path == '/')
            path++;
        char *slash = strchr(path, '/');
        u64 length;
        if (slash)
            length = slash - path;
        else
            length = UINT64_MAX;

        trie_node_t *child = find_child(current, path, length);
        if (child)
            current = child;
        else
            break;

        path += strlen(child->comp);
    }

    *out = current->root_vn;
    return (char *)path;
}

int vfs_mount_vnode(vnode_t *vn, const char *path)
{
    const char *_path = path;
    trie_node_t *current = &g_trie_root;

    while (*path != '\0')
    {
        while(*path == '/')
            path++;

        if (!*path)
            break;

        char *slash = strchr(path, '/');
        u64 comp_len = slash ? slash - path : VFS_MAX_NAME_LEN;

        trie_node_t *child = find_child(current, path, comp_len);
        if (child == NULL)
        {
            child = heap_alloc(sizeof(trie_node_t));
            strncpy(child->comp, path, comp_len);
            child->children_cnt = 0;
            current->children[current->children_cnt++] = child;
        }
        current = child;

        path += comp_len;
    }
    current->root_vn = vn;

    log("Vnode mounted at `%s`.", _path);
    return EOK;
}

int vfs_mount(block_device_t *blk, filesystem_type_t *fs_type, const char *path)
{
    if (fs_type->probe(blk))
    {
        vnode_t *vn;
        fs_type->get_root_vnode(blk, &vn);
        return vfs_mount_vnode(vn, path);
    }
    else
        return -1;
}

void vfs_register_fs_type(filesystem_type_t *fs_type)
{
    spinlock_acquire(&g_fs_type_list_slock);

    list_append(&g_fs_type_list, &fs_type->list_node);

    spinlock_release(&g_fs_type_list_slock);
}

/*
 * Veneer layer
 */

int vfs_open(const char *path, vnode_t **out)
{
    vnode_t *curr;
    path = vfs_get_mountpoint(path, &curr);
    ASSERT(curr != NULL);
    while (curr && *path)
    {
        while(*path == '/')
            path++;

        char *slash = strchr(path, '/');
        if (slash)
            *slash = '\0';
        curr->ops->open(curr, path, &curr);
        if (slash)
            *slash = '/';

        path += strlen(curr->name);
    }

    *out = curr;
    return EOK;
}

int vfs_close(vnode_t *vn)
{
    rbtree_node_t *node = rb_first(&vn->page_cache);
    while (node)
    {
        vnode_page_t *vp = RBTREE_GET_CONTAINER(node, vnode_page_t, rbtree_node);

        u64 wrote_bytes = 0;
        int st = vn->ops->write(
            vn,
            vp->rbtree_node.key,
            (void*)vp->page_addr,
            ARCH_PAGE_GRAN,
            &wrote_bytes
        );
        if (st != EOK)
            log("Failed to flush page when closing vnode!");

        node = rb_next(node);

        rb_remove(&vn->page_cache, vp->rbtree_node.key);
        pmm_free((void *)vp->page_addr);
        heap_free(vp);
    }

    if (vn->ops->close)
        return vn->ops->close(vn);

    return EOK;
}

int vfs_read(vnode_t *vn, u64 offset, void *buffer, u64 count, u64 *out)
{
    if (!vn->cached)
        return vn->ops->read(
            vn,
            offset,
            buffer,
            count,
            out
        );

    u64 done = 0;
    while (done < count)
    {
        u64 off_in_page = (offset + done) % ARCH_PAGE_GRAN;
        u64 page_off    = (offset + done) - off_in_page;
        u64 to_copy     = ARCH_PAGE_GRAN - off_in_page;
        if (to_copy > (count - done))
            to_copy = count - done;

        rbtree_node_t *node = rb_find(&vn->page_cache, page_off);
        vnode_page_t *vp = NULL;
        if (node)
            vp = RBTREE_GET_CONTAINER(node, vnode_page_t, rbtree_node);
        else
        {
            vp = heap_alloc(sizeof(vnode_page_t));
            if (!vp)
                return -ENOMEM;
            *vp = (vnode_page_t) {
                .page_addr = (uptr)pmm_alloc(0),
                .rbtree_node.key = page_off
            };

            u64 read_bytes = 0;
            int st = vn->ops->read(
                vn,
                page_off,
                (void*)vp->page_addr + HHDM,
                ARCH_PAGE_GRAN,
                &read_bytes
            );
            if (st != EOK)
            {
                heap_free(vp);
                return st;
            }

            rb_insert(&vn->page_cache, &vp->rbtree_node);
        }

        memcpy(
            (u8 *)buffer + done,
            (void *)(vp->page_addr + off_in_page + HHDM),
            to_copy
        );

        done += to_copy;
    }

    if (out)
        *out = done;
    return EOK;
}

int vfs_write(vnode_t *vn, u64 offset, void *buffer, u64 count, u64 *out)
{
    if (!vn->cached)
        return vn->ops->write(
            vn,
            offset,
            buffer,
            count,
            out
        );

    u64 done = 0;
    while (done < count)
    {
        u64 off_in_page = (offset + done) % ARCH_PAGE_GRAN;
        u64 page_off    = (offset + done) - off_in_page;
        u64 to_copy     = ARCH_PAGE_GRAN - off_in_page;
        if (to_copy > (count - done))
            to_copy = count - done;

        rbtree_node_t *node = rb_find(&vn->page_cache, page_off);
        vnode_page_t *vp = NULL;
        if (node)
            vp = RBTREE_GET_CONTAINER(node, vnode_page_t, rbtree_node);
        else
        {
            vp = heap_alloc(sizeof(vnode_page_t));
            if (!vp)
                return -ENOMEM;
            *vp = (vnode_page_t) {
                .page_addr = (uptr)pmm_alloc(0),
                .rbtree_node.key = page_off
            };

            if (off_in_page > 0 || to_copy < ARCH_PAGE_GRAN)
            {
                u64 read_bytes = 0;
                vn->ops->read(
                    vn,
                    page_off,
                    (void *)vp->page_addr + HHDM,
                    ARCH_PAGE_GRAN,
                    &read_bytes
                );
            }

            rb_insert(&vn->page_cache, &vp->rbtree_node);
        }

        memcpy(
            (void *)(vp->page_addr + off_in_page + HHDM),
            (u8 *)buffer + done,
            to_copy
        );

        u64 wrote_bytes = 0;
        int st = vn->ops->write(
            vn,
            offset + done,
            (u8 *)buffer + done,
            to_copy,
            &wrote_bytes
        );
        if (st != EOK)
            return st;

        done += to_copy;
    }

    if (out)
        *out = done;

    return 0;
}

int vfs_create(const char *path, vnode_type_t t, vnode_t **out)
{
    char parent_path[256];
    char child_name[64];

    char *c = strrchr(path, '/');
    if (!c)
    {
        *out = NULL;
        return -EINVAL;
    }

    u64 parent_len = (u64)(c - path);
    memcpy(parent_path, path, parent_len); // Using strncpy would have not guaranteed null termination either way.
    parent_path[parent_len] = '\0';
    strcpy(child_name, c + 1);

    vnode_t *parent_node;
    if (vfs_open(parent_path, &parent_node) < 0)
    {
        *out = NULL;
        return -ENOENT;
    }

    int ret = parent_node->ops->create(parent_node, child_name, t, out);
    return ret;
}

int vfs_remove(const char *path)
{

}

/*
 * Initialization & Debug
 */

void vfs_init()
{
    strcpy(g_trie_root.comp, "/");
    g_trie_root.children_cnt = 0;

    log("VFS initialized");
}

static void _vfs_debug(trie_node_t *node, uint depth)
{
    char pad[16] = {0};
    for (uint i = 0; i < depth; i++)
        strcat(pad, " ");
    log("%s%s - %u", pad, node->comp, node->children_cnt);
    for (uint i = 0; i < node->children_cnt; i++)
        _vfs_debug(node->children[i], depth + 4);
}

void vfs_debug()
{
    _vfs_debug(&g_trie_root, 0);
}
