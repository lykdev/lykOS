#include <arch/thread.h>
#include <common/hhdm.h>
#include <lib/string.h>
#include <lib/math.h>
#include <mm/pmm.h>
#include <tasking/sched.h>
#include <sys/proc.h>
#include <sys/smp.h>

#include "tables/tss.h"
#include "fpu.h"
#include "msr.h"

extern void x86_64_thread_userspace_init();

extern uptr x86_64_abi_stack_setup(vmm_addr_space_t *as, size_t stack_size, char **argv, char **envp);

extern __attribute__((naked)) void x86_64_sched_context_switch(arch_thread_context_t *curr, arch_thread_context_t *next);

void arch_thread_context_init(arch_thread_context_t *context, proc_t *parent_proc, bool user, uptr entry)
{
    context->self = context;
    context->fs = context->gs = 0;

    if (!user)
    {
        context->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN;
        context->rsp = context->kernel_stack - sizeof(arch_thread_init_stack_kernel_t);
        memset((void *)context->rsp, 0, sizeof(arch_thread_init_stack_kernel_t));
        ((arch_thread_init_stack_kernel_t *)context->rsp)->entry = entry;
    }
    else
    {
        context->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN;
        context->rsp = context->kernel_stack - sizeof(arch_thread_init_stack_kernel_t);

        memset((void *)context->rsp, 0, sizeof(arch_thread_init_stack_user_t));
        ((arch_thread_init_stack_user_t *)context->rsp)->userspace_init = x86_64_thread_userspace_init;
        ((arch_thread_init_stack_user_t *)context->rsp)->entry = entry;

        char *argv[] = { "/usr/bin/init", NULL };
        char *envp[] = { NULL };
        ((arch_thread_init_stack_user_t *)context->rsp)->user_stack =
            x86_64_abi_stack_setup(parent_proc->addr_space, ARCH_PAGE_GRAN * 8, argv, envp);
    }

    u8 order = pmm_pagecount_to_order(CEIL(g_x86_64_fpu_area_size, ARCH_PAGE_GRAN)) / ARCH_PAGE_GRAN;
    context->fpu_area = (void*)((uptr)pmm_alloc(order) + HHDM);
    memset(context->fpu_area, 0, g_x86_64_fpu_area_size);

    if(g_smp_initialized)
        g_x86_64_fpu_save(sched_get_curr_thread()->context.fpu_area);
    g_x86_64_fpu_restore(context->fpu_area);

    u16 x87cw = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (0b11 << 8);
    asm volatile ("fldcw %0" : : "m"(x87cw) : "memory");
    u32 mxcsr = (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12);
    asm volatile ("ldmxcsr %0" : : "m"(mxcsr) : "memory");

    g_x86_64_fpu_save(context->fpu_area);
    if(g_smp_initialized)
           g_x86_64_fpu_restore(sched_get_curr_thread()->context.fpu_area);
}

void arch_thread_context_switch(arch_cpu_context_t *cpu, arch_thread_context_t *curr, arch_thread_context_t *next)
{
    // FPU
    g_x86_64_fpu_save(curr->fpu_area);
    g_x86_64_fpu_restore(next->fpu_area);
    // GS & FS
    curr->gs = x86_64_msr_read(X86_64_MSR_KERNEL_GS_BASE);
    curr->fs = x86_64_msr_read(X86_64_MSR_FS_BASE);
    x86_64_msr_write(X86_64_MSR_KERNEL_GS_BASE, next->gs);
    x86_64_msr_write(X86_64_MSR_FS_BASE, next->fs);
    // TSS -> stack pointer for when taking interrupts.
    x86_64_tss_set_rsp0(cpu->tss, next->kernel_stack);

    x86_64_sched_context_switch(curr, next); // This function calls `sched_drop` for `curr` too.
}
