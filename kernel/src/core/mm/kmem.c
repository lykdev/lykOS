#include "kmem.h"

#include <arch/cpu.h>

#include <core/mm/pmm.h>
#include <core/mm/vmm.h>
#include <core/tasking/def.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/string.h>
#include <utils/log.h>
#include <utils/panic.h>
#include <utils/printf.h>
#include <utils/limine/requests.h>

uint   g_cpu_count;
list_t g_cache_list = LIST_INIT;

static kmem_slab_t* kmem_new_slab(uint obj_size)
{
    kmem_slab_t *slab = (kmem_slab_t*)((uptr)pmm_alloc(pmm_pagecount_to_order(0x4000 /* 16 KIB */ / ARCH_PAGE_GRAN)) + HHDM);
    slab->obj_cnt = (0x4000 - sizeof(kmem_slab_t)) / obj_size;
    slab->obj_free = slab->obj_cnt;

    slab->freelist = NULL;
    uptr base = (uptr)slab + sizeof(slab);
    for (uint i = 0; i < slab->obj_cnt; i++)
    {
        void **obj = (void**)(base + i * obj_size);
        *obj = slab->freelist;
        slab->freelist = obj;
    }    

    return slab;
}

kmem_cache_t* kmem_new_cache(char *name, uint obj_size)
{
    kmem_cache_t *cache = (kmem_cache_t*)((uptr)pmm_alloc(0) + HHDM);
    strcpy(cache->name, name);
    cache->obj_size = obj_size;
    cache->slabs_partial = LIST_INIT;
    cache->slabs_full = LIST_INIT;
    cache->slab_list_lock = SLOCK_INIT;

    for (uint i = 0; i < g_cpu_count; i++)
        cache->per_cpu_active_slab[i] = kmem_new_slab(obj_size);

    list_append(&g_cache_list, &cache->list_elem);
}

void* kmem_alloc_from(kmem_cache_t *cache)
{
    uint curr_cpu_id = 0;
    // Before SMP is initialized all allocations will be assigned to the active slab of CPU core 0.
    if (g_smp_initialized)
        curr_cpu_id =  ((thread_t*)arch_cpu_read_thread_reg())->assigned_core->id;

    kmem_slab_t *active_slab = cache->per_cpu_active_slab[curr_cpu_id];
    void *ret = active_slab->freelist;
    active_slab->freelist = *(void**)active_slab->freelist;

    // If the active slab was filled swap it for a partially used/new one.
    if (active_slab->freelist == NULL)
    {
        slock_acquire(&cache->slab_list_lock);

        list_append(&cache->slabs_full, &active_slab->list_elem);
        if (list_is_empty(&cache->slabs_partial))
            cache->per_cpu_active_slab[curr_cpu_id] = kmem_new_slab(cache->obj_size);
        else
            cache->per_cpu_active_slab[curr_cpu_id] = LIST_GET_CONTAINER(list_pop_head(&cache->slabs_partial), kmem_slab_t, list_elem);

        slock_release(&cache->slab_list_lock);
    }

    return ret;
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
