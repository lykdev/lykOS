#include "gdt.h"

#include <common/log.h>

// Must be set to 1 for any valid segment.
#define ACCESS_PRESENT (1 << 7)
// Descriptor privilege level field. Contains the CPU Privilege level of the
// segment.
#define ACCESS_DPL(DPL) (((DPL) & 0b11) << 5)
#define ACCESS_TYPE_TSS (0b1001)
#define ACCESS_TYPE_CODE(CONFORM, READ)                                        \
  ((1 << 4) | (1 << 3) | ((CONFORM) << 2) | ((READ) << 1))
#define ACCESS_TYPE_DATA(DIRECTION, WRITE)                                     \
  ((1 << 4) | ((DIRECTION) << 2) | ((WRITE) << 1))

#define FLAG_GRANULARITY (1 << 7)
#define FLAG_DB (1 << 6)
#define FLAG_LONG (1 << 5)
#define FLAG_SYSTEM_AVL (1 << 4)

typedef struct
{
  u16 limit;
  u16 base_low;
  u8 base_mid;
  u8 access;
  u8 flags;
  u8 base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
  gdt_entry_t entry;
  u32 base_ext;
  u8 _rsv0;
  u8 _rsv1_zero;
  u16 _rsv2;
} __attribute__((packed)) gdt_system_entry_t;

typedef struct
{
  u16 limit;
  u64 base;
} __attribute__((packed)) gdt_descriptor_t;

extern void gdt_load(gdt_descriptor_t *gdtr, u64 selector_code,
                     u64 selector_data);

static gdt_entry_t g_gdt[] = {
    // Null descriptor.
    {},
    // Kernel code.
    {.limit = 0,
     .base_low = 0,
     .base_mid = 0,
     .access = ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_TYPE_CODE(0, 1),
     .flags = FLAG_LONG,
     .base_high = 0},
    // Kernel data.
    {.limit = 0,
     .base_low = 0,
     .base_mid = 0,
     .access = ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_TYPE_DATA(0, 1),
     .flags = 0,
     .base_high = 0},
    // User data.
    {.limit = 0,
     .base_low = 0,
     .base_mid = 0,
     .access = ACCESS_PRESENT | ACCESS_DPL(3) | ACCESS_TYPE_DATA(0, 1),
     .flags = 0,
     .base_high = 0},
    // User code.
    {.limit = 0,
     .base_low = 0,
     .base_mid = 0,
     .access = ACCESS_PRESENT | ACCESS_DPL(3) | ACCESS_TYPE_CODE(0, 1),
     .flags = FLAG_LONG,
     .base_high = 0},
    // TSS.
    {},
    {}};

void x86_64_gdt_load()
{
  gdt_descriptor_t gdtr =
      (gdt_descriptor_t){.limit = sizeof(g_gdt) - 1, .base = (u64)&g_gdt};

  log("Loading GDT(%#lx, %#lx)", gdtr.base, (u64)gdtr.limit);
  gdt_load(&gdtr, X86_64_GDT_SELECTOR_CODE64_RING0,
           X86_64_GDT_SELECTOR_DATA64_RING0);
}

void x86_64_gdt_load_tss(x86_64_tss_t *tss)
{
  u16 tss_segment = sizeof(g_gdt) - 16;

  gdt_system_entry_t *entry = (gdt_system_entry_t *)((uptr)g_gdt + tss_segment);
  entry->entry.access = ACCESS_PRESENT | ACCESS_TYPE_TSS;
  entry->entry.flags =
      FLAG_SYSTEM_AVL | ((sizeof(x86_64_tss_t) >> 16) & 0b1111);
  entry->entry.limit = (u16)sizeof(x86_64_tss_t);
  entry->entry.base_low = (u16)(u64)tss;
  entry->entry.base_mid = (u8)((u64)tss >> 16);
  entry->entry.base_high = (u8)((u64)tss >> 24);
  entry->base_ext = (u32)((u64)tss >> 32);

  asm volatile("ltr %0" : : "m"(tss_segment));
}
