#include "vfs.h"

#include <core/mm/kmem.h>

#include <utils/log.h>
#include <utils/string.h>

typedef struct trie_node_t trie_node_t;

struct trie_node_t
{
    /// @brief The path component separated by delimiting characters.
    char *token;
    /// @brief Pointer to the mountpoint, NULL if no mount here.
    vfs_mountpoint_t *mp;
    trie_node_t *children[16];
    uint children_cnt;
};

static trie_node_t g_root;

static trie_node_t* find_child(trie_node_t *parent, const char *token, uint n)
{
    for (uint i = 0; i < parent->children_cnt; i++)
        if (strncmp(parent->children[i]->token, token, n) == 0)
            return parent->children[i];
    return NULL;
}

int vfs_mount(const char *path, vfs_mountpoint_t *mp)
{
    trie_node_t *current = &g_root;
    const char *start = path;
    const char *end;

    if (*start++ != '/') // When mounting the given path must be absolute.
        return -1;
    while (*start != '\0')
    {
        // Find the next delimiter.
        end = start;
        while (*end != '/' && *end != '\0')
            end++;
        uint length = end - start;

        // Search for the token among the current node's children.
        trie_node_t *child = find_child(current, start, length);
        if (child == NULL)
        {
            child = kmem_alloc(sizeof(trie_node_t));
            child->token = kmem_alloc(length);
            strcpy(child->token, start);
            child->children_cnt = 0;

            current->children[current->children_cnt++] = child;
        }
        current = child;

        // Move to the next path token.
        start = (*end == '/') ? end + 1 : end;
    }
    current->mp = mp;
    return 0;
}

void vfs_init()
{
    g_root.token = kmem_alloc(8);
    *g_root.token = '\0';
    g_root.children_cnt = 0;
}

void _vfs_debug(trie_node_t *node, uint depth)
{
    char pad[16] = {0};
    for (uint i = 0; i < depth; i++)
        strcat(pad, " ");    
    log("%s%s - %u", pad, node->token, node->children_cnt);
    for (uint i = 0; i < node->children_cnt; i++)
        _vfs_debug(node->children[i], depth + 4);
}

void vfs_debug()
{
    _vfs_debug(&g_root, 0);
}
