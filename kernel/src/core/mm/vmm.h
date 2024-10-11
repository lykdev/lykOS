// #pragma once

// #include <utils/def.h>
// #include <utils/list.h>
// #include <utils/slock.h>

// typedef enum
// {
//     VMM_ANON,
// } vmm_seg_type_t;

// typedef struct
// {
//     vmm_address_space_t *addr_space;
//     vmm_seg_type_t type;
//     uptr base;
//     u64 length;

//     list_node_t list_elem;
// } vmm_seg_t;

// typedef struct
// {
//     slock_t slock;
//     list_t  segments;
//     uptr bound_low, bound_high;
// } vmm_addr_space_t;

// void* vmm_map(vmm_addr_space_t *addr_space, void *addr, u64 length, bool fixed, vmm_seg_type_t type);

// void vmm_init();
