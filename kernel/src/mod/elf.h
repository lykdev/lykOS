#pragma once

#include <fs/vfs.h>
#include <lib/list.h>

// typedef struct
// {
//     char *name;
//     char *author;
//     char *description;

//     void (*install)();
//     void (*destroy)();
//     void (*probe)();

//     vfs_node_t *mod_fs_node;
//     list_t memory_segments;
// }
// module_t;

// module_t *module_load(vfs_node_t *file);

elf_load_relocatable(vfs_node_t *file);