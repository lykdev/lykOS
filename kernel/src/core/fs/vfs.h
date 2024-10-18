#pragma once

typedef struct
{

}
vfs_mountpoint_t;

typedef enum
{
    VFS_NODE_FILE,
    VFS_NODE_DIR
}
vfs_node_type_t;

typedef struct
{   
    const char *name;
    vfs_node_type_t type;
}
vfs_node_t;
