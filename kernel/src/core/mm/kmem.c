#include "kmem.h"

#include <core/mm/pmm.h>
#include <core/mm/vmm.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/string.h>
#include <utils/log.h>
#include <utils/panic.h>
#include <utils/printf.h>
#include <utils/limine/requests.h>

uint   g_cpu_count;
list_t g_cache_list = LIST_INIT;

kmem_cache_t* kmem_new_cache(char *name, uint obj_size)
{
    kmem_cache_t *cache = (kmem_cache_t*)((uptr)pmm_alloc(0) + HHDM);
    strcpy(cache->name, name);
    cache->obj_size = obj_size;
    cache->slabs_partial = LIST_INIT;
    cache->slabs_full = LIST_INIT;

    for (uint i = 0; i < g_cpu_count; i++)
    {
        kmem_slab_t *slab = (kmem_slab_t*)((uptr)pmm_alloc(pmm_pagecount_to_order(0x4000 /* 16 KIB */ / ARCH_PAGE_GRAN)) + HHDM);
        cache->per_cpu_active_slab[i] = slab;
    }

    list_append(&g_cache_list, &cache->list_elem);
}

void* kmem_alloc_from(kmem_cache_t *cache)
{
    
}

void* kmem_alloc(uint size)
{
    FOREACH (n, g_cache_list)
    {
        kmem_cache_t *cache = LIST_GET_CONTAINER(n, kmem_cache_t, list_elem);

        if (cache->obj_size >= size)
            return kmem_alloc_from(cache);
    }
    // This should simply not happen unless we try allocate an obj too big.
    ASSERT_C(false, "Invalid obj size provided for kmem_alloc.");
}

void kmem_init()
{
    if (request_smp.response == NULL)
        panic("Invalid SMP info provided by the bootloader");
    g_cpu_count = request_smp.response->cpu_count;

    for (uint i = 8; i <= 512; i *= 2)
    {
        char name[32] = "slab-", buf[8];
        sprintf(buf, "%u", i);
        strcat(name, buf);

        kmem_new_cache(name, i);
    }
}
