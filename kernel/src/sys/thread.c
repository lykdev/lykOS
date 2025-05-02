#include "thread.h"

#include <arch/thread.h>
#include <arch/types.h>
#include <arch/x86_64/fpu.h>

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <sys/smp.h>
#include <tasking/sched.h>

/// @brief Last ID assigned to a thread.
static u64 g_last_id = 0;
list_t g_thread_list = LIST_INIT;

extern void x86_64_thread_userspace_init();

#include <arch/x86_64/abi/auxv.h>

extern uptr x86_64_abi_stack_setup(vmm_addr_space_t *as, size_t stack_size, char **argv, char **envp, x86_64_auxv_t *auxv);

thread_t *thread_new(proc_t *parent_proc, uptr entry)
{
    ASSERT(parent_proc != NULL);

    thread_t *thread = heap_alloc(sizeof(thread_t));
    *thread = (thread_t) {
#if defined(__x86_64__)
        .self = thread,
        .fpu_area = (void*)((uptr)pmm_alloc(2) + HHDM),
#endif
        .fs = 0, .gs = 0,
        .id = g_last_id++,
        .parent_proc = parent_proc,
        .assigned_core = NULL,
        .list_elem_thread = LIST_NODE_INIT,
        .list_elem_inside_proc = LIST_NODE_INIT,
        .mutex_wait_queue_node = LIST_NODE_INIT
    };

    if (parent_proc->type == PROC_KERNEL)
    {
        thread->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN - sizeof(arch_thread_init_stack_kernel_t);
        memset((void *)thread->kernel_stack, 0, sizeof(arch_thread_init_stack_kernel_t));
        ((arch_thread_init_stack_kernel_t *)thread->kernel_stack)->entry = entry;
    }
    else if (parent_proc->type == PROC_USER)
    {
        thread->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN - sizeof(arch_thread_init_stack_user_t);
        memset((void *)thread->kernel_stack, 0, sizeof(arch_thread_init_stack_user_t));
        ((arch_thread_init_stack_user_t *)thread->kernel_stack)->userspace_init = x86_64_thread_userspace_init;
        ((arch_thread_init_stack_user_t *)thread->kernel_stack)->entry = entry;

        char *argv[] = { "/usr/bin/init", NULL };
        char *envp[] = { NULL };
        ((arch_thread_init_stack_user_t *)thread->kernel_stack)->user_stack =
            x86_64_abi_stack_setup(parent_proc->addr_space, ARCH_PAGE_GRAN * 8, argv, envp);
    }

    //TODO: mask ints here
    memset(thread->fpu_area, 0, g_x86_64_fpu_area_size);

    if(g_smp_initialized)
        g_x86_64_fpu_save(sched_get_curr_thread()->fpu_area);
    g_x86_64_fpu_restore(thread->fpu_area);

    u16 x87cw = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (0b11 << 8);
    asm volatile ("fldcw %0" : : "m"(x87cw) : "memory");
    u32 mxcsr = (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12);
    asm volatile ("ldmxcsr %0" : : "m"(mxcsr) : "memory");

    g_x86_64_fpu_save(thread->fpu_area);
    if(g_smp_initialized)
        g_x86_64_fpu_restore(sched_get_curr_thread()->fpu_area);

    list_append(&parent_proc->threads, &thread->list_elem_inside_proc);

    return thread;
}
