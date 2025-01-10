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

#define SLAB_SIZE (64 * KIB)

static uint   g_cpu_count;
static list_t g_cache_list = LIST_INIT;

static kmem_slab_t* kmem_new_slab(kmem_cache_t *parent_cache)
{
    kmem_slab_t *slab = (kmem_slab_t*)((uptr)pmm_alloc(pmm_pagecount_to_order(SLAB_SIZE / ARCH_PAGE_GRAN)) + HHDM);
    uint _obj_cnt = (SLAB_SIZE - sizeof(kmem_slab_t)) / parent_cache->obj_size;
    
    *slab = (kmem_slab_t) {
        .parent_cache = parent_cache,
        .obj_cnt = _obj_cnt,
        .freelist = NULL,
        .freelist_len = _obj_cnt,
        .freelist_sec = NULL,
        .freelist_sec_len = 0,
        .lock = SLOCK_INIT,
        .assigned_cpu_id = -1
    };

    uptr base = (uptr)slab + sizeof(kmem_slab_t);
    for (uint i = 0; i < slab->obj_cnt; i++)
    {
        void **obj = (void**)(base + i * parent_cache->obj_size);
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
        cache->per_cpu_active_slab[i] = kmem_new_slab(cache);

    list_append(&g_cache_list, &cache->list_elem);

    return cache;
}

void* kmem_alloc_from(kmem_cache_t *cache)
{
    uint curr_cpu_id = 0;
    // Before SMP is initialized all allocations will be assigned to the active slab of CPU core 0.
    if (g_smp_initialized)
        curr_cpu_id = ((thread_t*)arch_cpu_read_thread_reg())->assigned_core->id;

    kmem_slab_t *active_slab = cache->per_cpu_active_slab[curr_cpu_id];

    // Because of the way this is implemented `ret` can be never NULL. Slabs are swapped in advance.
    void *ret = active_slab->freelist;
    active_slab->freelist = *(void**)active_slab->freelist;
    active_slab->freelist_len--;

    if (active_slab->freelist == NULL)
    {
        slock_acquire(&active_slab->lock);
        if (active_slab->freelist_sec != NULL)
        {
            active_slab->freelist = active_slab->freelist_sec;
            active_slab->freelist_len = active_slab->freelist_sec_len;

            active_slab->freelist_sec = NULL;
            active_slab->freelist_sec_len = 0;
        }
        else
        {
            slock_acquire(&cache->slab_list_lock);

            active_slab->assigned_cpu_id = -1;
            list_append(&cache->slabs_full, &active_slab->list_elem);

            if (list_is_empty(&cache->slabs_partial))
                cache->per_cpu_active_slab[curr_cpu_id] = kmem_new_slab(cache);
            else
                cache->per_cpu_active_slab[curr_cpu_id] = LIST_GET_CONTAINER(list_pop_head(&cache->slabs_partial), kmem_slab_t, list_elem);
            cache->per_cpu_active_slab[curr_cpu_id]->assigned_cpu_id = curr_cpu_id;

            slock_release(&cache->slab_list_lock);
        }
        slock_release(&active_slab->lock);
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
    return NULL;
}

void kmem_free(void *obj)
{
    int curr_cpu_id = 0;
    // Before SMP is initialized all allocations will be assigned to the active slab of CPU core 0.
    if (g_smp_initialized)
        curr_cpu_id = (int)((thread_t*)arch_cpu_read_thread_reg())->assigned_core->id;

    kmem_slab_t *slab = (kmem_slab_t*)((uptr)obj & (~(SLAB_SIZE - 1)));

    if (slab->assigned_cpu_id == -1) // Non-active slab.
    {
        slock_acquire(&slab->lock);

        *(void**)obj = slab->freelist;
        slab->freelist = obj;
        slab->freelist_len++;

        slock_acquire(&slab->parent_cache->slab_list_lock);
        if (slab->freelist_len == 1) // The slab was previously full and now is partial.
        {
            list_remove(&slab->parent_cache->slabs_full, &slab->list_elem);
            list_append(&slab->parent_cache->slabs_partial, &slab->list_elem);
        }
        else if (slab->freelist_len == slab->obj_cnt) // The slab was previously partial and now is empty.
        {
            list_remove(&slab->parent_cache->slabs_partial, &slab->list_elem);
            pmm_free((void*)((uptr)slab - HHDM));
        }
        else
            log("%u %u", slab->freelist_len, slab->obj_cnt);
        slock_release(&slab->parent_cache->slab_list_lock);

        slock_release(&slab->lock);
    }
    else if (slab->assigned_cpu_id == curr_cpu_id) // Active slab belonging to the current CPU.
    {
        *(void**)obj = slab->freelist;
        slab->freelist = obj;
        slab->freelist_len++;
    }
    else // Active slab belonging to another CPU.
    {
        slock_acquire(&slab->lock);

        *(void**)obj = slab->freelist_sec;
        slab->freelist_sec = obj;
        slab->freelist_sec_len++;

        slock_release(&slab->lock);
    }
}

void kmem_init()
{
    if (request_mp.response == NULL)
        panic("Invalid SMP info provided by the bootloader");
    g_cpu_count = request_mp.response->cpu_count;

    for (uint i = 8; i <= 512; i *= 2)
    {
        char name[32] = "cache-", buf[8];
        sprintf(buf, "%u", i);
        strcat(name, buf);

        kmem_new_cache(name, i);
    }

    log("KMEM init.");
}

void kmem_debug()
{
    FOREACH (n, g_cache_list)
    {
        kmem_cache_t *cache = LIST_GET_CONTAINER(n, kmem_cache_t, list_elem);

        log("%s - %u %u", cache->name, cache->slabs_partial.length, cache->slabs_full.length);
    }
}
