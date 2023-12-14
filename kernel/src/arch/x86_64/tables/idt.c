#include "idt.h"

#include <lib/utils.h>

#define IDT_ENTRY_FLAG_PRESENT 1 << 7
#define IDT_ENTRY_FLAG_INT 0b1110

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

struct int_regs
{
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 r11;
	u64 r10;
	u64 r9;
	u64 r8;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 rdx;
	u64 rcx;
	u64 rbx;
	u64 rax;
	u64 int_no;
	u64 err;
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} __attribute__((packed));

__attribute__((aligned(0x10)))
static struct idt_entry idt_entries[256];
static struct idt_desc idtr; 

void isr_handler(struct int_regs *regs)
{
    log("CPU EXCEPTION %d", regs->int_no);

    asm ("cli");
    for (;;)
        asm ("hlt"); 
}

static void idt_set_entry(u8 idx, uptr stub_addr)
{
    struct idt_entry *e = &idt_entries[idx];

    e->stub_low = stub_addr & 0xFFFF;
    e->kernel_cs = 0x28;
    e->ist = 0;
    e->flags = IDT_ENTRY_FLAG_PRESENT | IDT_ENTRY_FLAG_INT;
    e->stub_mid = (stub_addr >> 16) & 0xFFFF;
    e->stub_high = (stub_addr >> 32) & 0xFFFFFFFF;
    e->rsv = 0;
}

void idt_init()
{
    for (u64 i = 0; i < 32; i++)
        idt_set_entry(i, idt_stub_addr_table[i]);

    idtr.base = (u64)&idt_entries;
    idtr.limit = (u16)(sizeof(struct idt_entry) * 256 - 1);

    asm volatile("lidt %0" ::  "m"(idtr)); // Load the new IDT.
    asm volatile("sti"); // Set the interrupt flag.

    log("IDT initialized.");
}