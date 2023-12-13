#include <core/vmm.h>

#include <core/pmm.h>
#include <lib/utils.h>

#define PTE_VALID (1ull << 0ull)

#define PTE_TABLE (1ull << 1ull)
#define PTE_BLOCK (0ull << 1ull)

#define PTE_KERNEL (0ull << 6ull)
#define PTE_USER (1ull << 6ull)

#define PTE_RW (0ull << 7ull)
#define PTE_RO (1ull << 7ull)
#define PTE_NX (1ull << 54ull)

#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000FFFFFFFFFF000ull)

struct vmm_pagemap kernelmap;

static u64* vmm_get_next_level(u64 *top_level, u64 idx, bool alloc)
{   
    if (((top_level[idx]) & PTE_VALID) != 0)
        return (u64*)(PTE_GET_ADDR(top_level[idx]) + HHDM);

    if (!alloc)
        return NULL;
    
    void *next_level = pmm_get_page();
    if (next_level == NULL)
        panic("VMM: Out of memory.");
        
    memset(next_level, 0, PAGE_SIZE);

    top_level[idx] = (u64)(next_level - HHDM) | PTE_VALID | PTE_TABLE;
    return (u64*)next_level;
}

void vmm_map_page(struct vmm_pagemap *map, uptr virt, uptr phys, vmm_flags flags)
{
    u64 pml4e = (virt >> 39) & 0x1FF;
    u64 pml3e = (virt >> 30) & 0x1FF;
    u64 pml2e = (virt >> 21) & 0x1FF;
    u64 pml1e = (virt >> 12) & 0x1FF;

    u64 *pml4 = map->top_level;
    u64 *pml3 = vmm_get_next_level(pml4, pml4e, true);
    u64 *pml2 = vmm_get_next_level(pml3, pml3e, true);
    u64 *pml1 = vmm_get_next_level(pml2, pml2e, true);
    
    pml1[pml1e] = phys | PTE_RW | PTE_USER | PTE_VALID | PTE_BLOCK;
}

void vmm_setup_page_map(struct vmm_pagemap *map)
{
    map->top_level = (u64*)pmm_get_page();
    memset(map->top_level, 0, PAGE_SIZE);
}

void vmm_switch_to_map(struct vmm_pagemap *map)
{
    asm volatile("msr ttbr1_el1, %0" : : "r" (map));

    asm volatile("tlbi vmalle1is");

    asm volatile("dsb ish");
}

void vmm_init()
{
    log("%llx", KERNEL_ADDR_PHYS);
    log("%llx", KERNEL_ADDR_VIRT);
    log("%llx", HHDM);

    vmm_setup_page_map(&kernelmap);

    log("A");

    // Map the first 4GiB mandated by the limine spec.
    for (u64 i = 0; i < 4 * GIB; i += PAGE_SIZE)
        vmm_map_page(&kernelmap, HHDM + i, i, PTE_USER | PTE_RW | PTE_VALID);
        
    log("B");

    // Map the kernel to the last 2GiB of the virt addr space.
    for (u64 i = 0; i < 2 * GIB; i += PAGE_SIZE)
        vmm_map_page(&kernelmap, KERNEL_ADDR_VIRT + i, KERNEL_ADDR_PHYS + i, PTE_USER | PTE_RW | PTE_VALID);

    log("C");

    for (int i = 0; i < memmap_request.response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            for (uptr addr = entry->base; addr < entry->base + entry->length; addr += PAGE_SIZE)
                vmm_map_page(&kernelmap, HHDM + addr, addr, PTE_USER | PTE_RW | PTE_VALID);
        }
        else if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
        {
            for (uptr addr = entry->base; addr < entry->base + entry->length; addr += PAGE_SIZE)
                vmm_map_page(&kernelmap, HHDM + addr, addr, PTE_USER | PTE_RW | PTE_VALID);
        }
    }

    log("D");

    vmm_switch_to_map(&kernelmap);

    log("E");
}