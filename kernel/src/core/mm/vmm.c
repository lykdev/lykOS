#include "vmm.h"

#include <arch/types.h>

#include <core/mm/pmm.h>

#include <lib/log.h>
#include <lib/assert.h>
#include <lib/req.h>
#include <lib/hhdm.h>
#include <lib/mem.h>
#include <lib/slock.h>

#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITE   (1ull << 1ull)
#define PTE_USER    (1ull << 2ull)
#define PTE_HUGE    (1ull << 7ull)
#define PTE_NX      (1ull << 63ull)

#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000FFFFFFFFFF000ull)

vmm_map_t vmm_new_map()
{
    vmm_map_t map;

    map.pml4 = (pte_t*)(pmm_alloc(0)->phys_addr + HHDM);
    memset(map.pml4, 0, PAGE_SIZE_4KIB);

    map.slock = SLOCK_INIT;

    return map;
}

pte_t *vmm_get_next_level(pte_t *top_level, u64 idx, bool alloc)
{
    if (((top_level[idx]) & PTE_PRESENT) != 0)
        return (pte_t*)(PTE_GET_ADDR(top_level[idx]) + HHDM);

    if (!alloc)
        return NULL;
    
    pte_t *next_level = (pte_t*)(pmm_alloc(0)->phys_addr + HHDM);
    memset(next_level, 0, PAGE_SIZE_4KIB);

    top_level[idx] = (pte_t)((uptr)next_level - HHDM) | PTE_PRESENT | PTE_WRITE;
    return next_level;
} 

void vmm_map(vmm_map_t *map, uptr virt, uptr phys, page_size_t size)
{
    slock_acquire(&map->slock);

    pte_t pml4e = (virt >> 39) & 0x1FF;

    pte_t pml3e = (virt >> 30) & 0x1FF;
    if (size == PAGE_SIZE_1GIB)
        goto step2;

    pte_t pml2e = (virt >> 21) & 0x1FF;
    if (size == PAGE_SIZE_2MIB)
        goto step2;

    pte_t pml1e = (virt >> 12) & 0x1FF;

step2:

    pte_t *pml4 = map->pml4;

    pte_t *pml3 = vmm_get_next_level(pml4, pml4e, true);
    if (size == PAGE_SIZE_1GIB)
    {
        pml3[pml3e] = phys | PTE_PRESENT | PTE_WRITE | PTE_HUGE;
        goto end;
    }
        
    pte_t *pml2 = vmm_get_next_level(pml3, pml3e, true);
    if (size == PAGE_SIZE_2MIB)
    {
        pml2[pml2e] = phys | PTE_PRESENT | PTE_WRITE | PTE_HUGE;
        goto end;
    }

    pte_t *pml1 = vmm_get_next_level(pml2, pml2e, true);
    pml1[pml1e] = phys | PTE_PRESENT | PTE_WRITE;

end:

    slock_release(&map->slock);
}

void vmm_load_map(vmm_map_t *map)
{
    __asm__ volatile (
        "movq %0, %%cr3"
        :
        : "r" ((uptr)map->pml4 - HHDM)
        : "memory"
    );
}

vmm_map_t vmm_kernelmap;

void vmm_init()
{
    log("Kernel phys addr: %#llx", req_kernel_addr.response->physical_base);
    log("Kernel virt addr: %#llx", req_kernel_addr.response->virtual_base);
    log("HHDM: %#llx", HHDM);

    vmm_kernelmap = vmm_new_map();

    // Map the first 4GiB mandated by the limine spec.
    // We can use 1 GIB pages for this.
    for (uptr addr = 0; addr < 4 * GIB; addr += PAGE_SIZE_1GIB) 
        vmm_map(&vmm_kernelmap, HHDM + addr, addr, PAGE_SIZE_1GIB);

    // Map the kernel to the last 2GiB of the virt addr space.
    for (uptr addr = 0; addr < 2 * GIB; addr += PAGE_SIZE_4KIB)
        vmm_map(&vmm_kernelmap, req_kernel_addr.response->virtual_base + addr, req_kernel_addr.response->physical_base + addr, PAGE_SIZE_4KIB);

    for (usize i = 0; i < req_memmap.response->entry_count; i++)
    {
        struct limine_memmap_entry *e = req_memmap.response->entries[i];

        switch (e->type)
        {
        case LIMINE_MEMMAP_USABLE:
        case LIMINE_MEMMAP_FRAMEBUFFER:
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: // The stack is here lol.
            for (uptr addr = e->base; addr < e->base + e->length; addr += PAGE_SIZE_4KIB)
                vmm_map(&vmm_kernelmap, HHDM + addr, addr, PAGE_SIZE_4KIB);       
        break;
        }
    }

    vmm_load_map(&vmm_kernelmap);

    log("A new kernel pagemap has been generated and loaded.");
}