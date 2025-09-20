#include "vfs.h"

#include <common/assert.h>
#include <common/log.h>
#include <common/panic.h>
#include <lib/path.h>
#include <lib/string.h>
#include <mm/heap.h>

/*
 * Mount points
 */

typedef struct trie_node_t trie_node_t;

struct trie_node_t
{
    /// @brief The path component separated by delimiting characters.
    char comp[32];
    /// @brief Pointer to the mountpoint, NULL if no mount here.
    vfs_mountpoint_t *mp;
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

static char *vfs_get_mountpoint(const char *path, vfs_mountpoint_t **out)
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

    *out = current->mp;
    return (char *)path;
}

int vfs_mount(const char *path, vfs_mountpoint_t *mp)
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
    current->mp = mp;

    log("Filesystem mounted at %s.", _path);
    return 0;
}

/*
 * Veneer layer
 */

int vfs_open(const char *path, vnode_t **out)
{
    vfs_mountpoint_t *mp;
    path = vfs_get_mountpoint(path, &mp);
    ASSERT(mp != NULL);

    vnode_t *curr = mp->root_node;
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

int vfs_close(vnode_t *out)
{

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
