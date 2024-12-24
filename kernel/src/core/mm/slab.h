#pragma once

#include <utils/def.h>
#include <utils/list.h>

typedef struct kmem_cache kmem_cache_t;
typedef struct kmem_slab kmem_slab_t;

typedef struct kmem_cache
{
    const char name[32];
    uint obj_size;
    list_t slabs_partial;
    list_t slabs_full;
    list_node_t list_elem;
    kmem_slab_t per_cpu_active_slab[];
}
kmem_cache_t;

typedef struct kmem_slab
{
    void *freelist;
    list_node_t list_elem;
}
kmem_slab_t;

void kmem_init();
