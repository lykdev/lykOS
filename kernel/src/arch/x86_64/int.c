#include <arch/cpu.h>
#include <arch/int.h>

#include <arch/x86_64/devices/lapic.h>

#include <common/log.h>
#include <lib/def.h>
#include <mm/vmm.h>
#include <sys/proc.h>
#include <sys/thread.h>

typedef struct [[gnu::packed]] {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no;
    uint64_t err_code, rip, cs, rflags, rsp, ss;
} cpu_state_t;

typedef enum
{
    DIVISION_ERROR                  =  0,
    DEBUG                           =  1,
    NON_MASKABLE_INTERRUPT          =  2,
    BREAKPOINT                      =  3,
    OVERFLOW                        =  4,
    BOUND_RANGE_EXCEEDED            =  5,
    INVALID_OPCODE                  =  6,
    DEVICE_NOT_AVAILABLE            =  7,
    DOUBLE_FAULT                    =  8,
    COPROCESSOR_SEGMENT_OVERRUN     =  9,
    INVALID_TSS                     = 10,
    SEGMENT_NOT_PRESENT             = 11,
    STACK_SEGMENT_FAULT             = 12,
    GENERAL_PROTECTION_FAULT        = 13,
    PAGE_FAULT                      = 14,
    RESERVED_15                     = 15,
    X87_FLOATING_POINT_EXCEPTION    = 16,
    ALIGNMENT_CHECK                 = 17,
    MACHINE_CHECK                   = 18,
    SIMD_FLOATING_POINT_EXCEPTION   = 19,
    VIRTUALIZATION_EXCEPTION        = 20,
    CONTROL_PROTECTION_EXCEPTION    = 21,
    RESERVED_22                     = 22,
    RESERVED_23                     = 23,
    RESERVED_24                     = 24,
    RESERVED_25                     = 25,
    RESERVED_26                     = 26,
    RESERVED_27                     = 27,
    HYPERVISOR_INJECTION_EXCEPTION  = 28,
    VMM_COMMUNICATION_EXCEPTION     = 29,
    SECURITY_EXCEPTION              = 30,
    RESERVED_31                     = 31
}
x86_64_exception_code_t;

void (*handlers[256])() = { NULL };

void arch_int_handler(cpu_state_t *cpu_state)
{
    log("int %llx", cpu_state->int_no);
    if (cpu_state->int_no < 32)
    {
        bool handled = false;

        if (cpu_state->int_no == 14) // PF
        {
            uptr fault_addr;
            asm volatile ("mov %%cr2, %0" : "=r" (fault_addr));
            proc_t *proc = ((thread_t*)arch_cpu_read_thread_reg())->parent_proc;
            handled = vmm_pagefault_handler(proc->addr_space, fault_addr);
        }

        if (!handled)
        {
            log("CPU EXCEPTION: %llu %#llx", cpu_state->int_no, cpu_state->err_code);
            arch_cpu_halt();
        }
    }
    else
    {
        if (handlers[cpu_state->int_no] != NULL)
            handlers[cpu_state->int_no]();
    }

    x86_64_lapic_send_eoi();
}

void arch_int_register_exception_handler(uint exception, void (*handler)(void))
{
    handlers[exception] = handler;
}

void arch_int_register_irq_handler(uint irq, void (*handler)(void))
{
    handlers[32 + irq] = handler;
}
