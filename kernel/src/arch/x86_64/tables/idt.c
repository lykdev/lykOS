#include "idt.h"

#include <lib/utils.h>
#include <proc/sched.h>

#define IDT_ENTRY_FLAG_PRESENT 1 << 7
#define IDT_ENTRY_FLAG_INT 0b1110
#define IDT_ENTRY_FLAG_TRAP 0b1111

extern uptr idt_stub_addr_table[];

struct idt_entry
{
    u16 stub_low;
    u16 kernel_cs;
    u8 ist;
    u8 flags;
    u16 stub_mid;
    u32 stub_high;
    u32 rsv;
} __attribute__((packed));

struct idt_desc
{
	u16	limit;
	u64	base;
} __attribute__((packed));

__attribute__((aligned(0x10)))
static struct idt_entry idt_entries[256];
static struct idt_desc idtr; 

void isr_handler(struct int_regs *regs)
{
    panic("CPU EXCEPTION %d", regs->int_no);

    asm volatile ("cli");
    halt();
}

static uptr irq_handlers[8];

void irq_handler(struct int_regs *regs)
{
    void (*jump_addr)(struct int_regs *regs) = irq_handlers[regs->int_no - 32];
    jump_addr(regs);
}

static void idt_set_entry(u8 idx, uptr stub_addr, u8 flags)
{
    struct idt_entry *e = &idt_entries[idx];

    e->stub_low = stub_addr & 0xFFFF;
    e->kernel_cs = 0x28;
    e->ist = 0;
    e->flags = flags;
    e->stub_mid = (stub_addr >> 16) & 0xFFFF;
    e->stub_high = (stub_addr >> 32) & 0xFFFFFFFF;
    e->rsv = 0;
}

void idt_init()
{
    for (u64 i = 0; i < 32; i++)
        idt_set_entry(i, idt_stub_addr_table[i], IDT_ENTRY_FLAG_PRESENT | IDT_ENTRY_FLAG_TRAP);

    idtr.base = (u64)&idt_entries;
    idtr.limit = (u16)(sizeof(struct idt_entry) * 256 - 1);

    asm volatile("lidt %0" ::  "m"(idtr)); // Load the new IDT.
    asm volatile("sti"); // Set the interrupt flag.

    log("IDT initialized.");
}

void irq_register_handler(u8 idx, uptr handler)
{
    idt_set_entry(idx + 32, idt_stub_addr_table[idx + 32], IDT_ENTRY_FLAG_PRESENT | IDT_ENTRY_FLAG_INT);
    irq_handlers[idx] = handler;
}