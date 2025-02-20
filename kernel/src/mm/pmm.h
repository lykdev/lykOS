#pragma once

#include <arch/types.h>
#include <utils/def.h>

#define PMM_MAX_ORDER 10

void pmm_debug_info();

u8 pmm_pagecount_to_order(u64 pages);

u64 pmm_order_to_pagecount(u8 order);

void *pmm_alloc(u8 order);

void pmm_free(void *addr);

void pmm_init();
