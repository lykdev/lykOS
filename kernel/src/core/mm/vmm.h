#pragma once

#include <arch/ptm.h>

#include <utils/def.h>
#include <utils/list.h>
#include <utils/slock.h>

// Types

typedef struct
{
    slock_t slock;
    list_t segments;
    uptr limit_low;
    uptr limit_high;

    arch_ptm_map_t ptm_map;
}
vmm_addr_space_t;

typedef enum
{
    VMM_FIXED = 1 << 0,
}
vmm_flags_t;

typedef enum
{
    VMM_ANON,
}
vmm_seg_type_t;

typedef struct
{
    vmm_addr_space_t *addr_space;
    vmm_seg_type_t type;
    uptr base;
    u64  len;

    list_node_t list_elem;
}
vmm_seg_t;

// Public

extern vmm_addr_space_t vmm_kernel_addr_space;

uptr vmm_map(vmm_addr_space_t *addr_space, uptr virt, uptr phys, size_t len, vmm_seg_type_t type);

vmm_addr_space_t vmm_new_addr_space(uptr limit_low, uptr limit_high);

void vmm_init();
