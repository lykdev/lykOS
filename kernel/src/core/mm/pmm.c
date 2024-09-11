#include "pmm.h"

#include <arch/types.h>

#include <utils/def.h>
#include <utils/hhdm.h>
#include <utils/limine/requests.h>
#include <utils/list.h>
#include <utils/log.h>

typedef struct
{
    uptr addr;
    u8 order;
    bool free;

    list_node_t list_elem;
} pmm_block_t;

pmm_block_t *blocks;
u64 block_count;
list_t levels[PMM_MAX_ORDER + 1];

// UTILS

static u8 pagecount_to_order(size_t pages)
{
    if(pages == 1)
        return 0;
    return (u8)(64 - __builtin_clzll(pages - 1));
}

static u64 order_to_pagecount(u8 order)
{
    return (u64)1 << order;
}

void pmm_debug_info()
{
    log(2, "Free blocks per order:");
    for (int i = 0; i <= PMM_MAX_ORDER; i++)
        log(2, "Order %d: %llu", i, levels[i].length);

    u64 fram = 0;
    for (int i = 0; i <= PMM_MAX_ORDER; i++)
        fram += order_to_pagecount(i) * levels[i].length * PAGE_GRAN;
    
    log(2, "Free RAM: %lluMiB +  %lluKiB", fram / MIB, fram % MIB / KIB);
}

// 

void* pmm_alloc(u8 order)
{
    int i = order;

    while (list_is_empty(&levels[i]))
        i++;

    while (i != order)
    {
           
    }    
}

// INIT

void pmm_init()
{
    for (int i = 0; i <= PMM_MAX_ORDER; i++)
        levels[i] = LIST_INIT;    

    // Find the last usable memory entry to determine how many blocks our pmm should manage.
    struct limine_memmap_entry *last_usable_entry;
    for (u64 i = 0; i < request_memmap.response->entry_count; i++)
    {
        struct limine_memmap_entry *e = request_memmap.response->entries[i];

        // log(2, "%d %#llx %#llx - %lluMiB +  %lluKiB", e->type, e->base, e->length, e->length / MIB, e->length % MIB / KIB);

        if (e->type == LIMINE_MEMMAP_USABLE)
            last_usable_entry = e;
    }
    block_count = (last_usable_entry->base + last_usable_entry->length) / PAGE_GRAN;

    // Find a usable memory entry at the start of which the blocks array will be placed.
    for (u64 i = 0; i < request_memmap.response->entry_count; i++)
    {
        struct limine_memmap_entry *e = request_memmap.response->entries[i];
        if (e->type == LIMINE_MEMMAP_USABLE)
            if (e->length >= block_count * sizeof(pmm_block_t))
            {
                blocks = (pmm_block_t*)(e->base + HHDM);
                break;
            }                
    }

    // Set each block's address and mark them as used for now.
    for (u64 i = 0; i < block_count; i++)
        blocks[i] = (pmm_block_t) {
            .addr = PAGE_GRAN * i,
            .free = false
        };

    // Iterate through each entry and set the blocks corresponding to a usable memory entry as free using greedy.
    for (u64 i = 0; i < request_memmap.response->entry_count; i++)
    {
        struct limine_memmap_entry *e = request_memmap.response->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE)
            continue;

        u8 order = PMM_MAX_ORDER;
        uptr addr = e->base;
        while (addr != e->base + e->length)
        {
            u64 span = order_to_pagecount(order) * PAGE_GRAN;

            if (addr + span > e->base + e->length)
            {
                order--;
                continue;
            }

            blocks[addr / PAGE_GRAN] = (pmm_block_t) {
                .order = order,
                .free = true
            };
            list_append(&levels[order], &blocks[addr / PAGE_GRAN].list_elem);

            addr += span;
        }
    }

    pmm_debug_info();
}
