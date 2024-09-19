#include "pmm.h"

#include <arch/types.h>

#include <utils/assert.h>
#include <utils/def.h>
#include <utils/hhdm.h>
#include <utils/limine/requests.h>
#include <utils/list.h>
#include <utils/log.h>
#include <utils/string.h>

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

static u8 pagecount_to_order(u64 pages)
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
    {
        i++;   

        if (i > PMM_MAX_ORDER)
            panic("OUT OF MEMORY");
    }    
            
    pmm_block_t *block = LIST_GET_CONTAINER(levels[i].head, pmm_block_t, list_elem);
    list_remove(&levels[i], levels[i].head);
    
    while (i != order)
    {
        // Left block.
        u64 l_idx = block->addr / PAGE_GRAN;
        pmm_block_t *left = &blocks[l_idx];
        left->order = i - 1;
        left->free  = true;

        // Right block.
        u64 r_idx = l_idx ^ order_to_pagecount(i - 1);
        if (r_idx < block_count) // Check if within bounds.
        {
            pmm_block_t *right = &blocks[r_idx];
            right->order = i - 1;
            right->free = true;
            list_append(&levels[i - 1], &right->list_elem);
        }
        
        block = left;
        i--;
    }    

    block->order = order;
    block->free  = false;
    return (void*)block->addr;
}

void pmm_free(void *addr)
{
    u64 idx = (u64)addr / PAGE_GRAN;
    pmm_block_t *block = &blocks[idx];
    u8 i = block->order;

    while (i < PMM_MAX_ORDER)
    {
        u64 b_idx = idx ^ order_to_pagecount(i);
        if (b_idx >= block_count)
            break;

        pmm_block_t *buddy = &blocks[b_idx];
        if (buddy->free == true && buddy->order == i)
        {
            list_remove(&levels[buddy->order], &buddy->list_elem);

            // The new merged block is on the left.
            block = idx < b_idx ? block : buddy;
            idx = idx < b_idx ? idx : b_idx;
            i++;
        }
        else
            break;
    }    

    block->order = i;
    block->free  = true;
    list_append(&levels[i], &block->list_elem);
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
    memset(blocks, 0, sizeof(pmm_block_t) * block_count);
    for (u64 i = 0; i < block_count; i++)
        blocks[i] = (pmm_block_t) {
            .addr = PAGE_GRAN * i,
            .free = false,
            .list_elem = LIST_NODE_INIT
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

            u64 idx = addr / PAGE_GRAN;
            blocks[idx].order = order;
            blocks[idx].free  = true;
            list_append(&levels[order], &blocks[idx].list_elem);

            addr += span;
        }
    }
    
    pmm_debug_info();
}
