#include "heap.h"

#include <common/log.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <mm/kmem.h>

static kmem_cache_t g_caches[8];
static const size_t g_cache_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
static const char *g_cache_names[] = {"heap-8", "heap-16", "heap-32", "heap-64", "heap-128", "heap-256", "heap-512", "heap-1024"};

void *heap_alloc(size_t size)
{
    int order;
    if (size <= 8)
        order = 0;
    else
        order = 61 - __builtin_clzll(size - 1);

    return kmem_alloc_cache(&g_caches[order]);
}

void heap_free_size(void *obj, size_t size)
{
    int order;
    if (size <= 8)
        order = 0;
    else
        order = 60 - __builtin_clzll(size - 1);

    kmem_free_cache(&g_caches[order], obj);
}

void heap_free(void *obj)
{
    kmem_slab_t *slab = (kmem_slab_t*)((uptr)obj & (~(SLAB_SIZE - 1)));
    size_t size = slab->cache->object_size;

    heap_free_size(obj, size);
}

void *heap_realloc(void *obj, size_t old_size, size_t new_size)
{
    void *new_obj = heap_alloc(new_size);
    memcpy(new_obj, obj, old_size);

    return new_obj;
}

void heap_init()
{
    for (size_t size = 8, i = 0; size < 1024; size *= 2, i++)
        kmem_cache_intialize(&g_caches[i], g_cache_names[i], g_cache_sizes[i]);

    log("Heap initialized");
}
