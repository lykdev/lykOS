#pragma once

#include <lib/utils.h>

#define VMM_PRESENT 1
#define VMM_WRITE 1 << 1
#define VMM_USER 1 << 2
#define VMM_EXECUTE 1 << 3

typedef u64 vmm_flags;

struct vmm_pagemap
{
    u64 *top_level;
};

void vmm_init();
void vmm_setup_page_map(struct vmm_pagemap *map);
void vmm_map_page(struct vmm_pagemap *map, uptr virt, uptr phys, vmm_flags flags);
void vmm_switch_to_map(struct vmm_pagemap *map);
