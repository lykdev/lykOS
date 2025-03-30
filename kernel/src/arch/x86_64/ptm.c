#include <arch/ptm.h>

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <lib/string.h>
#include <mm/pmm.h>

#define PRESENT (1ull << 0)
#define WRITE (1ull << 1)
#define USER (1ull << 2)
#define HUGE (1ull << 7)
#define GLOBAL (1ull << 8)
#define NX (1ull << 63)

#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000FFFFFFFFFF000ull)

// GLOBAL DATA

static pte_t higher_half_entries[256];

// PTM LOGIC

static pte_t *get_next_level(pte_t *top_level, u64 idx, bool alloc)
{
    if (top_level[idx] & PRESENT)
        return (pte_t *)(PTE_GET_ADDR(top_level[idx]) + HHDM);

    if (!alloc)
        return NULL;

    pte_t *next_level = (pte_t *)((uptr)pmm_alloc(0) + HHDM);
    memset(next_level, 0, 0x1000);

    top_level[idx] = (pte_t)((uptr)next_level - HHDM) | PRESENT | WRITE | USER;
    return next_level;
}

static void delete_level(pte_t *lvl, u8 depth)
{
    if (depth != 1)
    {
        for (u64 i = 0; i < 512; i++)
        {
            if (!(lvl[i] & PRESENT) || lvl[i] & HUGE)
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

    pte_t *table = map->pml4;
    u64 i;
    for (i = 3; i >= 1; i--)
    {
        table = get_next_level(table, table_entries[i], true);
    }
    table[table_entries[i]] = phys | PRESENT | WRITE | USER;
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

    pte_t *table = map->pml4;
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
    __asm__ volatile("movq %0, %%cr3" : : "r"((uptr)map->pml4 - HHDM) : "memory");
}

arch_ptm_map_t arch_ptm_new_map()
{
    arch_ptm_map_t map;

    map.pml4 = (pte_t *)((uptr)pmm_alloc(0) + HHDM);
    memset(map.pml4, 0, ARCH_PAGE_GRAN);

    for (int i = 0; i < 256; i++)
        map.pml4[i + 256] = higher_half_entries[i];

    return map;
}

void arch_ptm_clear_map(arch_ptm_map_t *map) { delete_level(map->pml4, 4); }

uptr arch_ptm_virt_to_phys(arch_ptm_map_t *map, uptr virt)
{
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
        table = get_next_level(table, table_entries[i], false);
    }
    return PTE_GET_ADDR(table[table_entries[i]]) + (virt & 0xFFF);
}

void arch_ptm_init()
{
    for (int i = 0; i < 256; i++)
    {
        pte_t *table = (pte_t *)((uptr)pmm_alloc(0) + HHDM);
        memset(table, 0, ARCH_PAGE_GRAN);

        higher_half_entries[i] = (pte_t)((uptr)table - HHDM) | PRESENT | WRITE | GLOBAL;
    }
}
