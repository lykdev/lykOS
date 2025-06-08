#include "sched.h"

#include <arch/cpu.h>
#include <arch/thread.h>
#include <arch/timer.h>
#include <arch/int.h>
#include <common/log.h>
#include <common/panic.h>
#include <common/sync/spinlock.h>
#include <sys/smp.h>
#include <sys/thread.h>

static spinlock_t slock = SPINLOCK_INIT;
static list_t g_thread_list = LIST_INIT;

#include <common/assert.h>
#include <common/hhdm.h>

static thread_t *sched_next()
{
    spinlock_acquire(&slock);

    thread_t *ret = NULL;

    list_node_t *node = list_pop_head(&g_thread_list);
    if (node != NULL)
        ret = LIST_GET_CONTAINER(node, thread_t, list_elem_thread);
    else
        ret = sched_get_curr_thread()->assigned_core->idle_thread;

    spinlock_release(&slock);

    return ret;
}

thread_t *sched_get_curr_thread()
{
    return (thread_t*)arch_cpu_read_thread_reg();
}

void sched_drop(thread_t *thread)
{
    spinlock_acquire(&slock);

    if (thread != sched_get_curr_thread()->assigned_core->idle_thread
    &&  thread->status == THREAD_STATE_READY)
        list_append(&g_thread_list, &thread->list_elem_thread);

    spinlock_release(&slock);
}

void sched_queue_add(thread_t *thread)
{
    spinlock_acquire(&slock);

    list_append(&g_thread_list, &thread->list_elem_thread);

    spinlock_release(&slock);
}

void sched_yield(thread_status_t status)
{
    arch_cpu_int_mask();
    arch_timer_stop();

    thread_t *curr = sched_get_curr_thread();
    thread_t *next = sched_next();
    smp_cpu_core_t *cpu = curr->assigned_core;

    next->status = THREAD_STATE_RUNNING;
    arch_timer_oneshoot(2, 5 * 1'000);

    if (curr == next)
        return;

    next->assigned_core = curr->assigned_core;
    curr->assigned_core = NULL;
    curr->status = status;
    vmm_load_addr_space(next->parent_proc->addr_space);
    arch_cpu_write_thread_reg(next);

    /*
     * The implementation of this function is expected to call `sched_drop`
     * for the current thread and also unmask interrupts.
     */
    arch_thread_context_switch(&cpu->context, &curr->context, &next->context);
}

static void sched_preempt()
{
    sched_yield(THREAD_STATE_READY);
}

void sched_init()
{
    arch_int_register_irq_handler(2, sched_preempt);
}
