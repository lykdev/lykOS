#pragma once

#include <arch/types.h>
#include <utils/def.h>

#define PMM_MAX_ORDER 10

void pmm_debug_info();

void *pmm_alloc(u8 order);

void pmm_free(void *addr);

void pmm_init();
