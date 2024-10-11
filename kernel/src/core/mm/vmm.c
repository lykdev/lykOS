// #include "vmm.h"

// #include <core/mm/pmm.h>

// #define ADDR_IN_BOUNDS(ADDRESS_SPACE, ADDRESS) ((ADDRESS) >= (ADDRESS_SPACE)->start && (ADDRESS) < (ADDRESS_SPACE)->end)
// #define ADDR_IN_SEGMENT(ADDRESS, BASE, LENGTH) ((ADDRESS) >= (BASE) && (ADDRESS) < ((BASE) + (LENGTH)))

// #define SEG_IN_BOUNDS(ADDRESS_SPACE, BASE, LENGTH) (ADDRESS_IN_BOUNDS((ADDRESS_SPACE), (BASE)) && ((ADDRESS_SPACE)->end - (BASE)) >= (LENGTH))
// #define SEG_INTERSECTS(BASE1, LENGTH1, BASE2, LENGTH2) ((BASE1) < ((BASE2) + (LENGTH2)) && (BASE2) < ((BASE1) + (LENGTH1))

// list_t free_seg_list = LIST_INIT;

// vmm_seg_t* alloc_segment()
// {
//     if (list_is_empty(&free_seg_list))
//     {
//         vmm_seg_t *segments = (vmm_seg_t*)pmm_alloc(0) + HHDM;
//         u64 seg_count = PAGE_GRAN / sizeof(vmm_seg_t);

//         for (int i = 0; i < seg_count; i++)
//         {
//             segments[i].list_elem = LIST_NODE_INIT;
//             list_append(&free_seg_list, &segments[i].list_elem);
//         }
//     }
    
//     list_node_t *node = free_seg_list.head;
//     list_remove(&free_seg_list, node);

//     return LIST_GET_CONTAINER(node, vmm_seg_t, list_elem);
// }

// void *vmm_map(vmm_addr_space_t *addr_space, void *addr, u64 length, bool fixed, vmm_seg_type_t type)
// {
//     FOREACH (n, addr_space->segments)
//     {
//         vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

//         if (!SEG_IN_BOUNDS(addr_space, (uptr)addr, length))
//             return NULL;

//         if (seg->base >= (uptr)addr + length)
//             break;
        
//         if (SEG_INTERSECTS(addr, length, seg->base, seg->length))
//         {
//             if (fixed)
//                 return NULL;
//             addr = (void*)(seg->base + seg->length);    
//         }        
//     }

    

//     return NULL;
// }

// void vmm_init()
// {

// }
