#pragma  once

#include <lib/def.h>

void *heap_alloc(size_t size);

void heap_free_size(void *obj, size_t size);

void heap_free(void *obj);

void *heap_realloc(void *obj, size_t old_size, size_t new_size);

void heap_init();

__attribute__((unused))
static void _cleanup_free(void *p)
{
    heap_free(*(void**) p);
}

#define CLEANUP __attribute__((cleanup(_cleanup_free)))

#define CLEANUP_FUNC(func) __attribute__((cleanup(func)))
