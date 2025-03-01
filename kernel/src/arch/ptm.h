#pragma once

#include <arch/types.h>
#include <lib/def.h>

typedef u64 pte_t;

typedef struct
{
#if defined (__x86_64__)
    pte_t *pml4;
#elif defined (__aarch64__)
    pte_t *pml4[2];
#endif
} arch_ptm_map_t;

typedef enum
{
    ARCH_PTM_WRITE   = 1 << 0,
    ARCH_PTM_USER    = 1 << 1,
    ARCH_PTM_EXECUTE = 1 << 2
} arch_ptm_flags_t;

void arch_ptm_map(arch_ptm_map_t *map, uptr virt, uptr phys, u64 size);

uptr arch_ptm_virt_to_phys(arch_ptm_map_t *map, uptr virt);

void arch_ptm_load_map(arch_ptm_map_t *map);

arch_ptm_map_t arch_ptm_new_map();

void arch_ptm_clear_map(arch_ptm_map_t *map);

void arch_ptm_init();
