#pragma once

#include <lib/utils.h>

u64 pmm_get_total_ram();

u64 pmm_get_used_ram();

u64 pmm_get_free_ram();

void pmm_free(void *page);

void *pmm_alloc();

void pmm_init();
