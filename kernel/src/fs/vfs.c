#include "vfs.h"

#include <common/assert.h>
#include <common/log.h>
#include <common/panic.h>
#include <lib/path.h>
#include <lib/string.h>
#include <mm/heap.h>

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

static trie_node_t *find_child(trie_node_t *parent, const char *comp)
{
    for (uint i = 0; i < parent->children_cnt; i++)
        if (strcmp(parent->children[i]->comp, comp) == 0)
            return parent->children[i];
    return NULL;
}

static char *vfs_get_mountpoint(const char *path, vfs_mountpoint_t **out)
{
    trie_node_t *current = &g_trie_root;

    while (*path != '\0')
    {
        char comp[VFS_MAX_NAME_LEN];
        char *path_next = path_consume_comp(path, comp);

        if (*comp == '\0') // Handle empty components gracefully
            panic("CCC");

        trie_node_t *child = find_child(current, comp);
        if (child)
            current = child;
        else
            break;

        path = path_next;
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
        char comp[VFS_MAX_NAME_LEN];
        path = path_consume_comp(path, comp);

        // Search for the token among the current node's children.
        trie_node_t *child = find_child(current, comp);
        if (child == NULL)
        {
            child = heap_alloc(sizeof(trie_node_t));
            strcpy(child->comp, comp);
            child->children_cnt = 0;
            current->children[current->children_cnt++] = child;
        }
        current = child;
    }
    current->mp = mp;

    log("Filesystem mounted at %s.", _path);
    return 0;
}

vfs_node_t *vfs_lookup(const char *path)
{
    vfs_mountpoint_t *mp;
    path = vfs_get_mountpoint(path, &mp);
    ASSERT(mp != NULL);

    vfs_node_t *curr = mp->root_node;
    while (curr != NULL)
    {
        char comp[64] = "";
        path = path_consume_comp(path, comp);

        if (comp[0] != '\0')
            curr = curr->dir_ops->lookup(curr, comp);
        else
            return curr;
    }

    return NULL;
}

void vfs_init()
{
    strcpy(g_trie_root.comp, "");
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
