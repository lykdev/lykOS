#include "pmm.h"

#include <arch/types.h>

#include <lib/req.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/math.h>
#include <lib/hhdm.h>
#include <lib/slock.h>

static slock_t slock = SLOCK_INIT;
static list_t levels[MAX_ORDER];
static list_t free_regions = LIST_INIT;

static u64 usable_ram = 0, free_ram = 0, used_ram = 0;

u64 order_to_page_cnt(u8 order)
{
    return 1ull << order;
}

u8 page_cnt_to_order(u64 page_cnt)
{
    if(page_cnt == 1)
        return 0;
    return (u8) (64 - __builtin_clzll(page_cnt - 1));
}

void pmm_free(pmm_block_t *b)
{
    slock_acquire(&slock);

    b->free = true;

    used_ram -= order_to_page_cnt(b->order) * PAGE_SIZE_4KIB;
    free_ram += order_to_page_cnt(b->order) * PAGE_SIZE_4KIB;

    // MERGING

    while (true)
    {
        if (b->order == MAX_ORDER)
            break;

        // Find the buddy and check if it is free.

        u64 index = (b->phys_addr - b->parent_region->base) / PAGE_SIZE_4KIB; // The index in `pages` where the block struct is stored within the region.

        u64 gap = order_to_page_cnt(b->order); // Retrieve the buddy's index.
        u64 buddy_index = index ^ gap;
        if (buddy_index >= b->parent_region->page_count) // Assure the buddy's index is in bounds.
            break;

        pmm_block_t *buddy = &b->parent_region->pages[buddy_index]; // Check if the buddy is free.
        if (!buddy->free) 
            break;

        // The buddy which was previously free must be removed from the list.
        list_remove(&levels[buddy->order], &buddy->list_elem);
        
        if (index > buddy_index) // The resulted primary block struct is always on the left.
            b = buddy;
        b->order++;
    }

    list_append(&levels[b->order], &b->list_elem);

    slock_release(&slock);
}

pmm_block_t* pmm_alloc(u8 order)
{
    slock_acquire(&slock);

    ASSERT(order <= MAX_ORDER);

    // Find the smallest free block of order greater or equal than the one specified.
    u8 split_order = order;
    while (list_is_empty(&levels[split_order]))
    {
        split_order++;
        ASSERT_C(split_order <= MAX_ORDER, "Out of memory!");
    }        

    // Split the blocks.
    pmm_block_t *block = LIST_GET_CONTAINER(LIST_FIRST(&levels[split_order]), pmm_block_t, list_elem);
    list_remove(&levels[split_order], &block->list_elem);
    for (; split_order > order; split_order--)
    {
        // Get the buddy
        u64 block_index = (block->phys_addr - block->parent_region->base) / PAGE_SIZE_4KIB;
        u64 buddy_index = block_index + order_to_page_cnt(split_order - 1);

        pmm_block_t *buddy = &block->parent_region->pages[buddy_index];

        // Set buddy info
        buddy->order = split_order - 1;
        buddy->free  = true;

        // Add the buddy to the free list
        list_append(&levels[split_order - 1], &buddy->list_elem); 
    }

    slock_release(&slock);

    block->order = order;
    block->free  = false;

    used_ram += order_to_page_cnt(order) * PAGE_SIZE_4KIB;
    free_ram -= order_to_page_cnt(order) * PAGE_SIZE_4KIB;

    return block;
}

void pmm_init()
{
    ASSERT(req_memmap.response != NULL
        || req_memmap.response->entry_count > 0);

    // Initialize the levels' lists.
    for (u64 i = 0; i < MAX_ORDER; i++)
        levels[i] = LIST_INIT;

    for (u64 i = 0; i < req_memmap.response->entry_count; i++)
    {
        // Find a free region.
        struct limine_memmap_entry *e = req_memmap.response->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE)
            continue;

        // Set up a region.

        pmm_region_t *region = (pmm_region_t*)(e->base + HHDM); // Place the region struct at the start of the region.
        list_append(&free_regions, &region->list_elem);

        region->base = e->base;
        region->page_count = e->length / PAGE_SIZE_4KIB;    // How many pages are there within the region.

        // Calculate how many actual pages are occupied by the region struct and mark them as not being free.
        u64 occ_pages = CEIL(sizeof(pmm_region_t) + sizeof(pmm_page_t) * region->page_count, PAGE_SIZE_4KIB) / PAGE_SIZE_4KIB;
        for (u64 idx = 0; idx < occ_pages; idx++)
        {
            region->pages[idx] = (pmm_block_t) {.order = 0, .free = false};
            usable_ram += PAGE_SIZE_4KIB;
            used_ram   += PAGE_SIZE_4KIB;
        }

        // Cover the region using the biggest block possible using greedy. Start from the first free page.
        for (u64 idx = occ_pages; idx < region->page_count;)
        {
            u8 order = page_cnt_to_order(region->page_count - idx);
            if (order > 0)
                order--;
            if (order > MAX_ORDER)
                order = MAX_ORDER;

            u64 size = order_to_page_cnt(order);

            region->pages[idx] = (pmm_block_t) { .order = order, .free = true };
            list_append(&levels[order], &region->pages[idx].list_elem);

            usable_ram += size * PAGE_SIZE_4KIB;
            free_ram   += size * PAGE_SIZE_4KIB;
            
            idx += size;
        }   

        // The fields `parent_region` and `phys_addr` will never change for the entries in `pages` under a region.
        for (u64 idx = 0; idx < region->page_count; idx++)
        {
            region->pages[idx].parent_region = region;
            region->pages[idx].phys_addr     = region->base + idx * PAGE_SIZE_4KIB;
        }
    }

    log("Usable RAM: %llu MIB + %llu KIB", usable_ram / MIB, (usable_ram % MIB) / KIB);
}

void pmm_debug_info()
{
    log_warn("----------");
    for (u64 i = 0; i <= MAX_ORDER; i++)
    {
        log_warn("%llu - %llu", i, levels[i].length);
    }
    log_warn("----------");
}

u64 pmm_get_usable_ram()
{
    return usable_ram;
}

u64 pmm_get_free_ram()
{
    return free_ram;
}

u64 pmm_get_used_ram()
{
    return used_ram;
}