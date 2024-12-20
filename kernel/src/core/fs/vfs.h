// #pragma once

// typedef struct
// {

// }
// vfs_mountpoint_t;

// typedef enum
// {
//     VFS_NODE_FILE,
//     VFS_NODE_DIR
// }
// vfs_node_type_t;

// typedef struct
// {   
//     const char *name;
//     vfs_node_type_t type;

//     // FILE

//     /**
//      * @brief Reads the contents of a file.
//      * @returns `0` on success, `-errno` on failure.
//      */
//     int (*read)(vfs_node_t *self, u64 offset, u64 size, void *buffer);
    
//     /**
//      * @brief Writes to the contents of a file.
//      * @returns `0` on success, `-errno` on failure.
//      */
//     int (*write)(vfs_node_t *self, u64 offset, u64 size, void *buffer);

//     // DIRECTORY
    
//     /**
//      * @brief Look up a node by name.
//      * @param out Pointer to the found node, or `NULL` if not found.
//      * @returns `0` on success, `-errno` on failure.
//      */
//     int (*lookup)(vfs_node_t *self, char *name, vfs_node_t **out);

//     /**
//      * @brief List the next entry in a directory.
//      * @param index Pointer to the current directory entry index.
//      * @param out Pointer to the found directory entry, or `NULL` if no more entries.
//      * @returns `0` on success, `-errno` on failure.
//      * @note Parameter `index` will be automatically incremented.
//      */
//     int (*list)(vfs_node_t *self, int *index, char **out);
// }
// vfs_node_t;
