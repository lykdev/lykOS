#include "pmm.h"

#include <arch/types.h>

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <lib/def.h>
#include <lib/list.h>
#include <lib/string.h>

typedef struct
{
  uptr addr;
  u8 order;
  bool free;

  list_node_t list_elem;
} pmm_block_t;

static pmm_block_t *blocks;
static u64 block_count;
static list_t levels[PMM_MAX_ORDER + 1];

// UTILS

u8 pmm_pagecount_to_order(u64 pages)
{
  if (pages == 1)
    return 0;
  return (u8)(64 - __builtin_clzll(pages - 1));
}

u64 pmm_order_to_pagecount(u8 order) { return (u64)1 << order; }

void pmm_debug_info()
{
  log("Free blocks per order:");
  for (int i = 0; i <= PMM_MAX_ORDER; i++)
    log("Order %d: %llu", i, levels[i].length);

  u64 fram = 0;
  for (int i = 0; i <= PMM_MAX_ORDER; i++)
    fram += pmm_order_to_pagecount(i) * levels[i].length * ARCH_PAGE_GRAN;

  log("Free RAM: %lluMiB +  %lluKiB", fram / MIB, fram % MIB / KIB);
}

// ALLOC & FREE

void *pmm_alloc(u8 order)
{
  int i = order;
  while (list_is_empty(&levels[i]))
  {
    i++;

    if (i > PMM_MAX_ORDER)
      panic("OUT OF MEMORY");
  }

  pmm_block_t *block =
      LIST_GET_CONTAINER(levels[i].head, pmm_block_t, list_elem);
  list_remove(&levels[i], levels[i].head);

  for (; i > order; i--)
  {
    // Right block.
    u64 r_idx = (block->addr / ARCH_PAGE_GRAN) ^ pmm_order_to_pagecount(i - 1);
    pmm_block_t *right = &blocks[r_idx];
    right->order = i - 1;
    right->free = true;
    list_append(&levels[i - 1], &right->list_elem);
  }

  block->order = order;
  block->free = false;
  return (void *)block->addr;
}

void pmm_free(void *addr)
{
  ASSERT_C((uptr)addr < HHDM,
           "PMM functions operate with lower half memory addresses.");

  u64 idx = (u64)addr / ARCH_PAGE_GRAN;
  pmm_block_t *block = &blocks[idx];
  u8 i = block->order;

  ASSERT(block->free == false);

  while (i < PMM_MAX_ORDER)
  {
    u64 b_idx = idx ^ pmm_order_to_pagecount(i);
    if (b_idx >= block_count)
      break;

    pmm_block_t *buddy = &blocks[b_idx];
    if (buddy->free == true && buddy->order == i)
    {
      list_remove(&levels[buddy->order], &buddy->list_elem);

      // The new merged block is on the left.
      block = idx < b_idx ? block : buddy;
      idx = idx < b_idx ? idx : b_idx;
      i++;
    } else
      break;
  }

  block->order = i;
  block->free = true;
  list_append(&levels[i], &block->list_elem);
}

// INIT

void pmm_init()
{
  for (int i = 0; i <= PMM_MAX_ORDER; i++)
    levels[i] = LIST_INIT;

  // Find the last usable memory entry to determine how many blocks our pmm
  // should manage.
  struct limine_memmap_entry *last_usable_entry;
  for (u64 i = 0; i < request_memmap.response->entry_count; i++)
  {
    struct limine_memmap_entry *e = request_memmap.response->entries[i];

    // log(2, "%d %#llx %#llx - %lluMiB +  %lluKiB", e->type, e->base,
    // e->length, e->length / MIB, e->length % MIB / KIB);

    if (e->type == LIMINE_MEMMAP_USABLE)
      last_usable_entry = e;
  }
  block_count =
      (last_usable_entry->base + last_usable_entry->length) / ARCH_PAGE_GRAN;

  // Find a usable memory entry at the start of which the blocks array will be
  // placed.
  blocks = NULL; // Useful for the assert.
  for (u64 i = 0; i < request_memmap.response->entry_count; i++)
  {
    struct limine_memmap_entry *e = request_memmap.response->entries[i];
    if (e->type == LIMINE_MEMMAP_USABLE)
      if (e->length >= block_count * sizeof(pmm_block_t))
      {
        blocks = (pmm_block_t *)(e->base + HHDM);
        break;
      }
  }
  ASSERT(blocks != NULL);

  // Set each block's address and mark them as used for now.
  memset(blocks, 0, sizeof(pmm_block_t) * block_count);
  for (u64 i = 0; i < block_count; i++)
    blocks[i] = (pmm_block_t){
        .addr = ARCH_PAGE_GRAN * i, .free = false, .list_elem = LIST_NODE_INIT};

  // Iterate through each entry and set the blocks corresponding to a usable
  // memory entry as free using greedy.
  for (u64 i = 0; i < request_memmap.response->entry_count; i++)
  {
    struct limine_memmap_entry *e = request_memmap.response->entries[i];
    if (e->type != LIMINE_MEMMAP_USABLE)
      continue;

    u8 order = PMM_MAX_ORDER;
    uptr addr = e->base;
    // We don't want to mark as free the pages that contain the pmm block array.
    // Remember the block array is placed at the start of a free region.
    if (addr == (uptr)blocks - HHDM)
      addr += (block_count * sizeof(pmm_block_t) + (ARCH_PAGE_GRAN - 1)) /
              ARCH_PAGE_GRAN * ARCH_PAGE_GRAN;
    while (addr != e->base + e->length)
    {
      u64 span = pmm_order_to_pagecount(order) * ARCH_PAGE_GRAN;

      if (addr + span > e->base + e->length || addr % span != 0)
      {
        order--;
        continue;
      }

      u64 idx = addr / ARCH_PAGE_GRAN;
      blocks[idx].order = order;
      blocks[idx].free = true;
      list_append(&levels[order], &blocks[idx].list_elem);

      addr += span;

      order = PMM_MAX_ORDER;
    }
  }

  log("PMM initialized.");
}
