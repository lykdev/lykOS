#pragma once

#include <utils/def.h>
#include <utils/list.h>

typedef struct
{
    void *freelist;
    list_node_t list_elem;
}
kmem_slab_t;

typedef struct
{
    char name[32];
    uint obj_size;
    list_t slabs_partial;
    list_t slabs_full;
    list_node_t list_elem;
    kmem_slab_t *per_cpu_active_slab[];
}
kmem_cache_t;

void kmem_init();
