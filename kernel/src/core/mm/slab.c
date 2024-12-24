#include "slab.h"

#include <core/mm/pmm.h>
#include <core/mm/vmm.h>

#include <utils/assert.h>
#include <utils/string.h>
#include <utils/limine/requests.h>

uint   g_cpu_count;
list_t g_cache_list = LIST_INIT;

kmem_cache_t* kmem_new_cache(const char *name, uint obj_size)
{
    ASSERT(slab_size % ARCH_PAGE_GRAN == 0);

    kmem_cache_t *cache = pmm_alloc(0);
    strcpy(cache->name) = name;
    cache->obj_size = obj_size;
    cache->slabs_partial = LIST_INIT;
    cache->slabs_full = LIST_INIT;

    for (uint i = 0; i < g_cpu_count; i++)
    {
        kmem_slab_t *slab = pmm_alloc(pmm_pagecount_to_order(0x4000 /* 16 KIB */ / ARCH_PAGE_GRAN));
        cache->per_cpu_active_slab[i] = slab;
    }

    list_append(&g_cache_list, &cache->list_elem);
}

void kmem_init()
{
    if (request_smp.response == NULL)
        panic("Invalid SMP info provided by the bootloader");
    g_cpu_count = request_smp.response->cpu_count;

    for (uint i = 8; i <= 512; i *= 2)
    {
        char name[32] = "slab-";
        str
        kmem_new_cache(, i)
    }

    pmm_alloc(0);
}
