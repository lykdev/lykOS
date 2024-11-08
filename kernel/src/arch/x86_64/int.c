#include <arch/int.h>

#include <arch/cpu.h>
#include <utils/def.h>
#include <utils/log.h>

typedef struct
{
	u16 isr_low;
	u16 kernel_cs;
	u8	ist;
	u8  flags;
	u16 isr_mid;
	u32 isr_high;
	u32 _rsv;
}
__attribute__((packed)) idt_entry_t;

typedef struct
{
	u16	limit;
	u64	base;
}
__attribute__((packed)) idtr_t;

typedef struct
{
    u64 rax;
    u64 rbx;
    u64 rcx;
    u64 rdx;
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u64 int_no;
    u64 err_code;
}
__attribute__((packed)) cpu_state_t;

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256];

extern uptr arch_int_stub_table[256];

void arch_int_unmask()
{
    asm volatile ("sti");
}

void arch_int_mask()
{
    asm volatile ("cli");
}

void arch_int_handler(cpu_state_t *cpu_state)
{
    if (cpu_state->int_no < 32)
    {
        log("CPU EXCEPTION: %llu %#llx", cpu_state->int_no, cpu_state->err_code);

        arch_cpu_halt();
    }
}

void arch_int_init()
{
    arch_int_mask();

    for (int i = 0; i < 256; i++)
    {
        u64 isr = (u64)arch_int_stub_table[i];

        idt[i] = (idt_entry_t) {
            .isr_low    = isr & 0xFFFF,
            .kernel_cs  = 0x28, // TODO: Change this
            .ist        = 0,
            .flags      = 0x8E,
            .isr_mid    = (isr >> 16) & 0xFFFF,
            .isr_high   = (isr >> 32) & 0xFFFFFFFF,
            ._rsv       = 0
        };
    }

    idtr_t idtr = (idtr_t) {
        .limit = sizeof(idt) - 1,
        .base  = (u64)&idt
    };
    asm volatile ("lidt %0" : : "m"(idtr)); 

    arch_int_unmask();
    log("IDT loaded.");
}
