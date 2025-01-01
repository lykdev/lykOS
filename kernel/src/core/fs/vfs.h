#pragma once

#include <utils/def.h>
#include <utils/slock.h>

typedef struct vfs_node vfs_node_t;
typedef struct vfs_mountpoint vfs_mountpoint_t;
typedef struct vfs_node_ops vfs_node_ops_t;

struct vfs_mountpoint
{
    vfs_node_t *root_node; 
};

typedef enum
{
    VFS_NODE_FILE,
    VFS_NODE_DIR
}
vfs_node_type_t;

struct vfs_node
{   
    vfs_node_type_t type;
    vfs_node_ops_t *ops;
    void *mp_node;
};

struct vfs_node_ops
{
    /**
     * @brief Reads the contents of a file.
     * @returns `0` on success, `-errno` on failure.
     */
    int (*read)(vfs_node_t *self, u64 offset, u64 size, void *buffer);
    
    /**
     * @brief Writes to the contents of a file.
     * @returns `0` on success, `-errno` on failure.
     */
    int (*write)(vfs_node_t *self, u64 offset, u64 size, void *buffer);
    
    /**
     * @brief Look up a node by name.
     * @param out Pointer to the found node, or `NULL` if not found.
     * @returns `0` on success, `-errno` on failure.
     */
    int (*lookup)(vfs_node_t *self, char *name, vfs_node_t **out);

    /**
     * @brief List the next entry in a directory.
     * @param index Pointer to the current directory entry index.
     * @param out Pointer to the found directory entry, or `NULL` if no more entries.
     * @returns `0` on success, `-errno` on failure.
     * @note Parameter `index` will be automatically incremented.
     */
    int (*list)(vfs_node_t *self, uint *index, char **out);
};

int vfs_mount(const char *path, vfs_mountpoint_t *mp);

int vfs_lookup(const char *path, vfs_node_t **out);

void vfs_init();

void vfs_debug();
