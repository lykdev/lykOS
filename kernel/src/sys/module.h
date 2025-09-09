#pragma once

#include <fs/vfs.h>
#include <lib/list.h>

typedef struct
{
    bool (*probe)();
    void (*install)();
    void (*destroy)();

    const char *name;
    const char *version;
    const char *description;
    const char *author;

    spinlock_t slock;
}
module_t;

extern list_t g_mod_module_list;

module_t *module_load(vnode_t *file);

#define MODULE_NAME(name)        [[gnu::used]] static const char __module_name[]        = name;
#define MODULE_VERSION(ver)      [[gnu::used]] static const char __module_version[]     = ver;
#define MODULE_DESCRIPTION(desc) [[gnu::used]] static const char __module_description[] = desc;
#define MODULE_AUTHOR(auth)      [[gnu::used]] static const char __module_author[]      = auth;

/*
 * Static array of dependency names used by the kernel to ensure that required
 * modules are loaded before this one.
 */
#define MODULE_DEP(...) \
    [[gnu::used]] static const char *__module_deps[]   = { __VA_ARGS__ }; \
    [[gnu::used]] static const int __module_deps_count = sizeof(__module_deps) / sizeof(__module_deps[0]);
