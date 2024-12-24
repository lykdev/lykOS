#include "heap.h"

#include <core/mm/pmm.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/list.h>
#include <utils/slock.h>

typedef struct
{
    bool free;
    u64 size;
    list_node_t list_elem;
}
block_hdr_t;

static list_t blocks = LIST_INIT;

void* heap_alloc(size_t size)
{
    if (size < 8)
        size = 8;

    FOREACH(n, blocks)
    {
        block_hdr_t *b = LIST_GET_CONTAINER(n, block_hdr_t, list_elem);
        if (b->free && b->size >= size)
        {
            b->free = false;

            // Split.
            if (b->size >= size + sizeof(block_hdr_t) + 8)
            {
                block_hdr_t *s = (block_hdr_t*)((uptr)b + sizeof(block_hdr_t) + size);
                s->size = b->size - size - sizeof(block_hdr_t);
                s->free = true;
                list_insert_after(&blocks, &b->list_elem, &s->list_elem);

                b->size = size;
            }

            return (void*)((uptr)b + sizeof(block_hdr_t));
        }
    }

    // In case no fit block was found expand the heap.
    ASSERT_C(size < ARCH_PAGE_GRAN - sizeof(block_hdr_t), "You're not supposed to allocate blocks this big with the heap.");
    block_hdr_t *b = (block_hdr_t*)((uptr)pmm_alloc(0) + HHDM);
    *b = (block_hdr_t) {
        b->free = false,
        b->size = ARCH_PAGE_GRAN,
        b->list_elem = LIST_NODE_INIT
    };
    list_append(&blocks, &b->list_elem);

    if (b->size >= size + sizeof(block_hdr_t) + 8)
    {
        block_hdr_t *s = (block_hdr_t*)((uptr)b + sizeof(block_hdr_t) + size);
        s->size = b->size - size - sizeof(block_hdr_t);
        s->free = true;
        list_insert_after(&blocks, &b->list_elem, &s->list_elem);

        b->size = size;
    }

    return (void*)((uptr)b + sizeof(block_hdr_t));
}

void heap_init()
{

}
