#include "vmm.h"

#include <mm/pmm.h>
#include <lib/utils.h>

#if defined (__x86_64__)

#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITE (1ull << 1ull)
#define PTE_USER (1ull << 2ull)
#define PTE_NX (1ull << 63ull)

#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000FFFFFFFFFF000ull)

struct vmm_pagemap kernelmap;

static u64 translate_flags(u64 flags)
{
    u64 ret = 0;

    if (flags & VMM_PRESENT)
        ret |= PTE_PRESENT;
    if (flags & VMM_WRITE)
        ret |= PTE_WRITE;
    if (flags & VMM_USER)
        ret |= PTE_USER;
    if (flags & VMM_NOEXE)
        ret |= PTE_NX;

    return ret;
}

static u64* vmm_get_next_level(u64 *top_level, u64 idx, bool alloc)
{   
    if (((top_level[idx]) & PTE_PRESENT) != 0)
        return (u64*)(PTE_GET_ADDR(top_level[idx]) + HHDM);

    if (!alloc)
        return NULL;
    
    void *next_level = pmm_alloc() + HHDM;
    if (next_level == NULL)
        panic("VMM: Out of memory.");
        
    memset(next_level, 0, PAGE_SIZE);

    top_level[idx] = (u64)(next_level - HHDM) | PTE_PRESENT | PTE_WRITE | PTE_USER;
    return (u64*)next_level;
}

void vmm_map_page(struct vmm_pagemap *map, uptr virt, uptr phys, u64 flags)
{
    slock_acquire(&map->lock);

    u64 pml4e = (virt >> 39) & 0x1FF;
    u64 pml3e = (virt >> 30) & 0x1FF;
    u64 pml2e = (virt >> 21) & 0x1FF;
    u64 pml1e = (virt >> 12) & 0x1FF;

    u64 *pml4 = map->top_level;
    u64 *pml3 = vmm_get_next_level(pml4, pml4e, true);
    u64 *pml2 = vmm_get_next_level(pml3, pml3e, true);
    u64 *pml1 = vmm_get_next_level(pml2, pml2e, true);
    
    pml1[pml1e] = phys | translate_flags(flags);

    slock_release(&map->lock);
}

void vmm_new_pagemap(struct vmm_pagemap *map)
{
    map->top_level = (u64*)(pmm_alloc() + HHDM);
    memset(map->top_level, 0, PAGE_SIZE);

    map->lock = SLOCK_INIT;
}

void vmm_switch_to_map(struct vmm_pagemap *map)
{
    asm volatile (
        "movq %0, %%cr3"
        :
        : "r" ((uptr)map->top_level - HHDM)
        : "memory"
    );
}

#endif

void vmm_init()
{
    log("KERNEL_ADDR_PHYS: %llx", KERNEL_ADDR_PHYS);
    log("KERNEL_ADDR_VIRT: %llx", KERNEL_ADDR_VIRT);
    log("HHDM: %llx", HHDM);

    vmm_new_pagemap(&kernelmap);

    log("A");

    //Map the first 4GiB mandated by the limine spec.
    for (u64 i = 0x1000; i < 4 * GIB; i += PAGE_SIZE) 
        vmm_map_page(&kernelmap, HHDM + i, i, VMM_WRITE | VMM_PRESENT);
        
    log("B");

    // Map the kernel to the last 2GiB of the virt addr space.
    for (u64 i = 0; i < 2 * GIB; i += PAGE_SIZE)
        vmm_map_page(&kernelmap, KERNEL_ADDR_VIRT + i, KERNEL_ADDR_PHYS + i, VMM_WRITE | VMM_PRESENT);

    log("C");

    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
            for (uptr addr = entry->base; addr < entry->base + entry->length; addr += PAGE_SIZE)
                vmm_map_page(&kernelmap, HHDM + addr, addr, VMM_WRITE | VMM_PRESENT);
        else if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
            for (uptr addr = entry->base; addr < entry->base + entry->length; addr += PAGE_SIZE)
                vmm_map_page(&kernelmap, HHDM + addr, addr, VMM_WRITE | VMM_PRESENT);
    }

    log("D");

    vmm_switch_to_map(&kernelmap);

    log("A new kernel pagemap has been generated and loaded.");
}