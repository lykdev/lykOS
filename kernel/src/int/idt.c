#include "idt.h"

#include <lib/utils.h>

#define IDT_ENTRY_FLAG_PRESENT 1 << 7
#define IDT_ENTRY_FLAG_INT 0b1110

extern void int_stub_0();
extern void int_stub_1();
extern void int_stub_2();
extern void int_stub_3();
extern void int_stub_4();
extern void int_stub_5();
extern void int_stub_6();
extern void int_stub_7();
extern void int_stub_8();
extern void int_stub_9();
extern void int_stub_10();
extern void int_stub_11();
extern void int_stub_12();
extern void int_stub_13();
extern void int_stub_14();
extern void int_stub_15();
extern void int_stub_16();
extern void int_stub_17();
extern void int_stub_18();
extern void int_stub_19();
extern void int_stub_20();
extern void int_stub_21();
extern void int_stub_22();
extern void int_stub_23();
extern void int_stub_24();
extern void int_stub_25();
extern void int_stub_26();
extern void int_stub_27();
extern void int_stub_28();
extern void int_stub_29();
extern void int_stub_30();
extern void int_stub_31();

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

struct idt_entry idt[256];
struct idt_desc idtr; 

void INTHandler(struct int_regs *regs)
{
    log("INT %d", regs->int_no);

    asm ("cli");
    for (;;)
        asm ("hlt"); 
}

static void idt_set_entry(struct idt_entry *e, void (*handler)())
{
    e->stub_low = (uint64_t)handler & 0xFFFF;
    e->kernel_cs = 0x28;
    e->ist = 0;
    e->flags = IDT_ENTRY_FLAG_PRESENT | IDT_ENTRY_FLAG_INT;
    e->stub_mid = ((uint64_t)handler >> 16) & 0xFFFF;
    e->stub_high = ((uint64_t)handler >> 32) & 0xFFFFFFFF;
    e->rsv = 0;
}

void idt_init()
{
    idt_set_entry(&idt[0], &int_stub_0);
    idt_set_entry(&idt[1], &int_stub_1);
    idt_set_entry(&idt[2], &int_stub_2);
    idt_set_entry(&idt[3], &int_stub_3);
    idt_set_entry(&idt[4], &int_stub_4);
    idt_set_entry(&idt[5], &int_stub_5);
    idt_set_entry(&idt[6], &int_stub_6);
    idt_set_entry(&idt[7], &int_stub_7);
    idt_set_entry(&idt[8], &int_stub_8);
    idt_set_entry(&idt[9], &int_stub_9);
    idt_set_entry(&idt[10], &int_stub_10);
    idt_set_entry(&idt[11], &int_stub_11);
    idt_set_entry(&idt[12], &int_stub_12);
    idt_set_entry(&idt[13], &int_stub_13);
    idt_set_entry(&idt[14], &int_stub_14);
    idt_set_entry(&idt[15], &int_stub_15);
    idt_set_entry(&idt[16], &int_stub_16);
    idt_set_entry(&idt[17], &int_stub_17);
    idt_set_entry(&idt[18], &int_stub_18);
    idt_set_entry(&idt[19], &int_stub_19);
    idt_set_entry(&idt[20], &int_stub_20);
    idt_set_entry(&idt[21], &int_stub_21);
    idt_set_entry(&idt[22], &int_stub_22);
    idt_set_entry(&idt[23], &int_stub_23);
    idt_set_entry(&idt[24], &int_stub_24);
    idt_set_entry(&idt[25], &int_stub_25);
    idt_set_entry(&idt[26], &int_stub_26);
    idt_set_entry(&idt[27], &int_stub_27);
    idt_set_entry(&idt[28], &int_stub_28);
    idt_set_entry(&idt[29], &int_stub_29);
    idt_set_entry(&idt[30], &int_stub_30);
    idt_set_entry(&idt[31], &int_stub_31);

    idtr.base = (u64)&idt[0];
    idtr.limit = (u16)(sizeof(struct idt_entry) * 256 - 1);

    asm volatile("lidt %0"
                    :
                    : "m"(idtr)); // Load the new IDT.
    asm volatile("sti"); // Set the interrupt flag.

    log("IDT initialized.");
}