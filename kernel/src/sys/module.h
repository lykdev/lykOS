#pragma once

#include <fs/vfs.h>
#include <lib/list.h>

typedef struct
{
    void (*install)();
    void (*destroy)();
    bool (*probe)();

    vnode_t *mod_fs_node;
}
module_t;

extern list_t g_mod_module_list;

module_t *module_load(vnode_t *file);
