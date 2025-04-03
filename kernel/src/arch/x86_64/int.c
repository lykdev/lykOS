#include <arch/cpu.h>
#include <arch/int.h>

#include <common/log.h>
#include <lib/def.h>
#include <mm/vmm.h>
#include <sys/proc.h>
#include <sys/thread.h>

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
    u64 cr2;
    u64 int_no;
    u64 err_code;
} __attribute__((packed)) cpu_state_t;

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
    if (cpu_state->int_no < 32)
    {
        if (cpu_state->int_no == 14) // PF
        {
            proc_t *proc = ((thread_t*)arch_cpu_read_thread_reg())->parent_proc;
            vmm_pagefault_handler(proc->addr_space, cpu_state->cr2);
        }

        log("CPU EXCEPTION: %llu %#llx", cpu_state->int_no, cpu_state->err_code);

        arch_cpu_halt();
    }
    else if (handlers[cpu_state->int_no - 32] != NULL)
        handlers[cpu_state->int_no - 32]();
}

void arch_int_irq_register_handler(uint irq, void (*handler)())
{
    handlers[irq] = handler;
}
