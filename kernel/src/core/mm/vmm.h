#pragma once

#include <arch/types.h>
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

extern vmm_addr_space_t *g_vmm_kernel_addr_space;

uptr vmm_map_anon(vmm_addr_space_t *addr_space, uptr virt, size_t len);

uptr vmm_map_direct(vmm_addr_space_t *addr_space, uptr virt, size_t len, uptr phys);

uptr vmm_virt_to_phys(vmm_addr_space_t *addr_space, uptr virt);

vmm_addr_space_t* vmm_new_addr_space(uptr limit_low, uptr limit_high);

void vmm_load_addr_space(vmm_addr_space_t *addr_space);

void vmm_init();
