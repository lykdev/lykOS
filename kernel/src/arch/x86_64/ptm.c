#include <arch/ptm.h>

#include <core/mm/pmm.h>
#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/log.h>
#include <utils/string.h>

#include <utils/limine/requests.h>

#define PRESENT (1ull <<  0)
#define WRITE   (1ull <<  1)
#define USER    (1ull <<  2)
#define HUGE    (1ull <<  7)
#define GLOBAL  (1ull <<  8)
#define NX      (1ull << 63)

#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000FFFFFFFFFF000ull)

// GLOBAL DATA

pte_t higher_half_entries[256];

// PTM LOGIC

pte_t *vmm_get_next_level(pte_t *top_level, u64 idx, bool alloc)
{
    if ((top_level[idx] & PRESENT) != 0)
        return (pte_t*)(PTE_GET_ADDR(top_level[idx]) + HHDM);

    if (!alloc)
        return NULL;
    
    pte_t *next_level = (pte_t*)((uptr)pmm_alloc(0) + HHDM);
    memset(next_level, 0, 0x1000);

    top_level[idx] = (pte_t)((uptr)next_level - HHDM) | PRESENT | WRITE;
    return next_level;
} 

void arch_ptm_map(arch_ptm_map_t *map, uptr virt, uptr phys, u64 size)
{
    ASSERT(virt % size == 0);
    ASSERT(phys % size == 0);

    u64 table_entries[] = {
        (virt >> 12) & 0x1FF, // PML1 entry
        (virt >> 21) & 0x1FF, // PML2 entry
        (virt >> 30) & 0x1FF, // PML3 entry
        (virt >> 39) & 0x1FF  // PML4 entry
    };

    pte_t *table = map->pml4;
    u64 i;
    for (i = 3; i >= 1; i--)
    {
        table = vmm_get_next_level(table, table_entries[i], true);
    }
    table[table_entries[i]] = phys | PRESENT | WRITE;
}

void arch_ptm_load_map(arch_ptm_map_t *map)
{
    __asm__ volatile (
        "movq %0, %%cr3"
        :
        : "r" ((uptr)map->pml4 - HHDM)
        : "memory"
    );
}

arch_ptm_map_t arch_ptm_new_map()
{
    arch_ptm_map_t map;

    map.pml4 = (pte_t*)((uptr)pmm_alloc(0) + HHDM);
    memset(map.pml4, 0, ARCH_PAGE_GRAN);

    for (int i = 0; i < 256; i++)
        map.pml4[i + 256] = higher_half_entries[i];

    return map;
}

arch_ptm_map_t vmm_kernelmap;

void arch_ptm_init()
{
    for (int i = 0; i < 256; i++)   
        higher_half_entries[i] = (pte_t)pmm_alloc(0) | PRESENT | WRITE;

    vmm_kernelmap = arch_ptm_new_map();
    // Map the first 4GiB of memory as mandated by the limine specification.
    for (uptr addr = 0; addr < 4 * GIB; addr += ARCH_PAGE_SIZE_4K) 
        arch_ptm_map(&vmm_kernelmap, HHDM + addr, addr, ARCH_PAGE_SIZE_4K);
    // Map the kernel to the last 2GiB of the virt addr space.
    for (uptr addr = 0; addr < 2 * GIB; addr += ARCH_PAGE_SIZE_4K)
        arch_ptm_map(&vmm_kernelmap, request_kernel_addr.response->virtual_base + addr, request_kernel_addr.response->physical_base + addr, ARCH_PAGE_SIZE_4K);

    arch_ptm_load_map(&vmm_kernelmap);
}
