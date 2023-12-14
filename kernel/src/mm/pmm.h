#pragma once

#include <lib/utils.h>

u64 pmm_get_total_ram();

u64 pmm_get_used_ram();

u64 pmm_get_free_ram();

void pmm_free_page(void *page);

void *pmm_get_page();

void pmm_init();
