#pragma once

#include <arch/types.h>
#include <utils/def.h>

typedef u64 pte_t;

typedef struct
{
#if defined (__x86_64__)
    pte_t *top_level;
#endif
} ptm_map_t;

typedef enum
{
    PTM_WRITE   = 1 << 0,
    PTM_USER    = 1 << 1,
    PTM_EXECUTE = 1 << 2
} ptm_flags_t;

bool arch_ptm_map(ptm_map_t *map, uptr virt, uptr phys, u64 size, u64 flags);
