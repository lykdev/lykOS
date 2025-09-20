#include <arch/ptm.h>

#include <mm/pmm.h>
#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <lib/string.h>

#define PRESENT (1ull << 0)
#define TABLE (1ull << 1)
#define BLOCK (0ull << 1)
#define PAGE_4K (1ull << 1)
#define USER (1ull << 6)
#define READONLY (1ull << 6)
#define ACCESS (1ull << 10)
#define XN (1ull << 54)

#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000FFFFFFFFFF000ull)

// GLOBAL DATA

static pte_t *higher_half_pml4;

// PTM LOGIC

static pte_t *get_next_level(pte_t *top_level, u64 idx, bool alloc)
{
    if ((top_level[idx] & PRESENT) != 0)
        return (pte_t *)(PTE_GET_ADDR(top_level[idx]) + HHDM);

    if (!alloc)
        return NULL;

    pte_t *next_level = (pte_t *)((uptr)pmm_alloc(0) + HHDM);
    memset(next_level, 0, 0x1000);

    top_level[idx] = (pte_t)((uptr)next_level - HHDM) | PRESENT | TABLE;
    return next_level;
}

static void delete_level(pte_t *lvl, u8 depth)
{
    if (depth != 1)
    {
        for (u64 i = 0; i < 512; i++)
        {
            //                         For huge blocks.
            if (!(lvl[i] & PRESENT) || (((lvl[i] >> 1) & 1) == 0))
                continue;

            delete_level((pte_t *)(PTE_GET_ADDR(lvl[i]) + HHDM), depth - 1);
        }
    }
    pmm_free((void *)((uptr)lvl - HHDM));
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

    bool is_higher_half = virt & (1ull << 63);
    pte_t *table = map->pml4[is_higher_half];
    u64 i;
    for (i = 3; i >= 1; i--)
    {
        table = get_next_level(table, table_entries[i], true);
    }
    table[table_entries[i]] = phys | PRESENT | PAGE_4K | ACCESS;
}

void arch_ptm_unmap(arch_ptm_map_t *map, uptr virt, uptr phys, u64 size)
{
    ASSERT(virt % size == 0);
    ASSERT(phys % size == 0);

    u64 table_entries[] = {
        (virt >> 12) & 0x1FF, // PML1 entry
        (virt >> 21) & 0x1FF, // PML2 entry
        (virt >> 30) & 0x1FF, // PML3 entry
        (virt >> 39) & 0x1FF  // PML4 entry
    };

    bool is_higher_half = virt & (1ull << 63);
    pte_t *table = map->pml4[is_higher_half];
    u64 i;
    for (i = 3; i >= 1; i--)
    {
        table = get_next_level(table, table_entries[i], false);
        if (table == NULL)
            return;
    }
    table[table_entries[i]] = 0;
}

void arch_ptm_load_map(arch_ptm_map_t *map)
{
    asm volatile("msr ttbr0_el1, %0\n"
                 "msr ttbr1_el1, %1\n"
                 "isb              \n"
                 :
                 : "r"((uptr)map->pml4[0] - HHDM), "r"((uptr)map->pml4[1] - HHDM)
                 : "memory");
}

arch_ptm_map_t arch_ptm_new_map()
{
    arch_ptm_map_t map;

    map.pml4[0] = (pte_t *)((uptr)pmm_alloc(0) + HHDM);
    memset(map.pml4[0], 0, ARCH_PAGE_GRAN);

    map.pml4[1] = higher_half_pml4;

    return map;
}

void arch_ptm_clear_map(arch_ptm_map_t *map)
{
    delete_level(map->pml4[0], 4);
    // We don't want to clear map->pml4[1] which is common to all maps.
}

uptr arch_ptm_virt_to_phys(arch_ptm_map_t *map, uptr virt)
{
    u64 table_entries[] = {
        (virt >> 12) & 0x1FF, // PML1 entry
        (virt >> 21) & 0x1FF, // PML2 entry
        (virt >> 30) & 0x1FF, // PML3 entry
        (virt >> 39) & 0x1FF  // PML4 entry
    };

    bool is_higher_half = virt & (1ull << 63);
    pte_t *table = map->pml4[is_higher_half];
    u64 i;
    for (i = 3; i >= 1; i--)
    {
        table = get_next_level(table, table_entries[i], false);
    }
    return PTE_GET_ADDR(table[table_entries[i]]) + (virt & 0xFFF);
}

void arch_ptm_init()
{
    higher_half_pml4 = (pte_t *)((uptr)pmm_alloc(0) + HHDM);
    memset(higher_half_pml4, 0, ARCH_PAGE_GRAN);
}
