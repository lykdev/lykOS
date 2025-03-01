#include "idt.h"

#include <arch/int.h>

#include <common/log.h>
#include <lib/def.h>

typedef struct {
  u16 isr_low;
  u16 kernel_cs;
  u8 ist;
  u8 flags;
  u16 isr_mid;
  u32 isr_high;
  u32 _rsv;
} __attribute__((packed)) idt_entry_t;

typedef struct {
  u16 limit;
  u64 base;
} __attribute__((packed)) idtr_t;

__attribute__((aligned(0x10))) static idt_entry_t idt[256];
extern uptr arch_int_stub_table[256];

void arch_int_init() {
  for (int i = 0; i < 256; i++) {
    u64 isr = (u64)arch_int_stub_table[i];

    idt[i] = (idt_entry_t){.isr_low = isr & 0xFFFF,
                           .kernel_cs = 0x28, // TODO: Change this
                           .ist = 0,
                           .flags = 0x8E,
                           .isr_mid = (isr >> 16) & 0xFFFF,
                           .isr_high = (isr >> 32) & 0xFFFFFFFF,
                           ._rsv = 0};
  }

  log("IDT generated.");
}

void x86_64_idt_load() {
  idtr_t idtr = (idtr_t){.limit = sizeof(idt) - 1, .base = (u64)&idt};
  asm volatile("lidt %0" : : "m"(idtr));
}
