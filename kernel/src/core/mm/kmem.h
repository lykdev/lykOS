#pragma once

#include <utils/def.h>
#include <utils/list.h>
#include <utils/slock.h>

typedef struct
{
    void *freelist;
    uint obj_cnt;
    uint obj_free;
    list_node_t list_elem;
}
kmem_slab_t;

typedef struct
{
    char name[32];
    uint obj_size;
    list_t slabs_partial;
    list_t slabs_full;
    slock_t slab_list_lock;
    list_node_t list_elem;
    kmem_slab_t *per_cpu_active_slab[];
}
kmem_cache_t;

kmem_cache_t *kmem_new_cache(char *name, uint obj_size);

void *kmem_alloc_from(kmem_cache_t *cache);

void* kmem_alloc(uint size);

void kmem_init();
