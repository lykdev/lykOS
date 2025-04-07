#pragma once

#include <fs/vfs.h>
#include <lib/list.h>

typedef struct
{
    char *name;
    char *author;
    char *description;

    void (*install)();
    void (*destroy)();
    void (*probe)();

    vfs_node_t *mod_fs_node;
}
module_t;

void mod_ksym_init();

uptr mod_resolve_sym(const char *name);

module_t *module_load(vfs_node_t *file);