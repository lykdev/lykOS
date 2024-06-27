#pragma once

#include <arch/types.h>

#include <lib/def.h>
#include <lib/slock.h>

/// @brief Page table entry.
typedef u64 pte_t;
/// @brief Page table flags.
typedef u64 pte_flags_t;

typedef struct
{
#if defined (__x86_64__)
    pte_t *pml4;
#else
    #error Unimplemented.
#endif    
    slock_t slock;
} vmm_map_t;

/// @brief The kernel's virtual memory map.
extern vmm_map_t vmm_kernelmap;

void vmm_init();

vmm_map_t vmm_new_map();

void vmm_map(vmm_map_t *map, uptr virt, uptr phys, page_size_t size);

void vmm_load_map(vmm_map_t *map);