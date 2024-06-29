#include "idt.h"

#include <arch/x86_64/gdt.h>

#include <lib/def.h>
#include <lib/log.h>

#define FLAGS_NORMAL 0x8E
#define FLAGS_TRAP   0x8F

typedef struct
{
    u16 offset_low;
    u16 segment_selector;
    u8  ist;
    u8  flags;
    u16 offset_mid;
    u32 offset_high;
    u32 _rsv;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
    u16 size;
    u64 base;
} __attribute__((packed)) idtr_t;

idt_entry_t idt[256];
extern uptr int_stubs[256];

void x86_64_int_handler(int_stack_frame_t *regs)
{
    if (regs->int_no < 32)
    {
        log_err("CPU exception occurred %d", regs->int_no);
        cpu_halt();
    }
}

static void set_gate(u8 gate, uptr handler, u16 segment, u8 flags)
{
    idt[gate].offset_low  = (u16) handler;
    idt[gate].offset_mid  = (u16) (handler >> 16);
    idt[gate].offset_high = (u32) (handler >> 32);
    idt[gate].segment_selector = segment;
    idt[gate].flags = flags;
    idt[gate].ist = 0;
    idt[gate]._rsv = 0;
}

void x86_64_idt_load()
{
    idtr_t idtr;
    idtr.size = sizeof(idt) - 1;
    idtr.base = (u64)&idt;

    for(u64 i = 0; i < 256; i++)
    {
        set_gate(i, int_stubs[i], X86_64_GDT_SEL_CODE_RING0, FLAGS_NORMAL);
    }

    asm volatile("lidt %0" : : "m" (idtr));

    log("IDT loaded.");
}