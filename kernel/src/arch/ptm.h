#pragma once

#include <arch/types.h>
#include <utils/def.h>

typedef u64 pte_t;

typedef struct
{
#if defined (__x86_64__)
    pte_t *pml4;
#endif
} arch_ptm_map_t;

typedef enum
{
    ARCH_PTM_WRITE   = 1 << 0,
    ARCH_PTM_USER    = 1 << 1,
    ARCH_PTM_EXECUTE = 1 << 2
} arch_ptm_flags_t;

bool arch_ptm_map(arch_ptm_map_t *map, uptr virt, uptr phys, u64 size, u64 flags);

void arch_ptm_load_map(arch_ptm_map_t *map);

arch_ptm_map_t arch_ptm_new_map();

