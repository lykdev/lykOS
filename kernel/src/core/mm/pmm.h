#pragma once

#include <lib/def.h>
#include <lib/list.h>

typedef struct pmm_block pmm_block_t;
typedef struct pmm_region pmm_region_t;

/// @brief Only to be used to reffer to order 0 blocks.
typedef pmm_block_t pmm_page_t;

struct pmm_block
{
    list_node_t list_elem;

    pmm_region_t *parent_region;
    uptr phys_addr;

    u8 order;
    bool free;
};

struct pmm_region
{
    list_node_t list_elem;

    uptr base;
    u64 page_count;
    u64 free_count;

    pmm_page_t pages[];
};

#define MAX_ORDER 10

void pmm_init();

void pmm_free(pmm_block_t *b);

pmm_block_t *pmm_alloc(u8 order);

u64 pmm_get_usable_ram();

u64 pmm_get_free_ram();

u64 pmm_get_used_ram();

void pmm_debug_info_levels();