#pragma once

#include <common/sync/spinlock.h>
#include <lib/def.h>
#include <lib/list.h>

#define MAG_SIZE 32
#define SLAB_SIZE 0x1000
#define MAX_CPUS 32

typedef struct
{
    list_node_t list_node;

    size_t count;
    void *objects[MAG_SIZE];
}
kmem_magazine_t;

typedef struct cpu_cache
{
    kmem_magazine_t *loaded;   // Currently loaded magazine.
    kmem_magazine_t *previous; // Previously loaded magazine.
}
kmem_cpu_cache_t;

typedef struct
{
    const char *name;
    size_t object_size;

    list_t slabs_full;      // List of full slabs.
    list_t slabs_partial;   // List of partial slabs.
    spinlock_t slabs_lock;

    list_t magazines_full;  // List of full magazines.
    list_t magazines_empty; // List of empty magazines.
    spinlock_t magazines_lock;

    kmem_cpu_cache_t cpu_cache[MAX_CPUS];
}
kmem_cache_t;

typedef struct
{
    list_node_t list_node;

    kmem_cache_t *cache;
    void *freelist;
}
kmem_slab_t;

kmem_cache_t *kmem_new_cache(const char *name, size_t size);

void kmem_cache_intialize(kmem_cache_t *cache, const char *name, size_t size);

void *kmem_alloc_cache(kmem_cache_t *cache);

void kmem_free_cache(kmem_cache_t *cache, void *obj);

void kmem_init();
