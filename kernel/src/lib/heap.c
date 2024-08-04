#include "heap.h"

#include <core/mm/pmm.h>

#include <lib/mem.h>
#include <lib/hhdm.h>

void *malloc(usize size)
{
    // Dumb temporary solution.
    return (void*)(pmm_alloc(0)->phys_addr + HHDM);
}

void *calloc(usize num, usize size)
{
    void *ret = malloc(num * size);
    memset(ret, 0, num * size);

    return ret;
}
