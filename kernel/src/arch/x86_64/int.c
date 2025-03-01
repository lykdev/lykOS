#include <arch/int.h>

#include <arch/cpu.h>
#include <lib/def.h>
#include <common/log.h>

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
