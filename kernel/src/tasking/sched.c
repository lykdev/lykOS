#include "sched.h"

#include <arch/cpu.h>

#include <common/limine/requests.h>
#include <common/log.h>
#include <common/panic.h>
#include <common/slock.h>
#include <sys/tasking.h>

extern __attribute__((naked)) void arch_sched_context_switch(thread_t *curr,
                                                             thread_t *next);

static slock_t slock = SLOCK_INIT;

static thread_t *sched_next() {
  thread_t *ret = NULL;

  slock_acquire(&slock);

  list_node_t *node = list_pop_head(&g_thread_list);
  if (node != NULL)
    ret = LIST_GET_CONTAINER(node, thread_t, list_elem_thread);

  slock_release(&slock);

  return ret;
}

void sched_drop(thread_t *thread) {
  if (thread !=
      ((thread_t *)arch_cpu_read_thread_reg())->assigned_core->idle_thread)
    sched_queue_add(thread);
}

void sched_queue_add(thread_t *thread) {
  slock_acquire(&slock);

  list_append(&g_thread_list, &thread->list_elem_thread);

  slock_release(&slock);
}

void sched_yield() {
  thread_t *curr = arch_cpu_read_thread_reg();
  thread_t *next = sched_next();

  if (next == NULL)
    next = ((thread_t *)arch_cpu_read_thread_reg())->assigned_core->idle_thread;
  if (curr != next) {
    next->assigned_core = curr->assigned_core;
    curr->assigned_core = NULL;
  }

  vmm_load_addr_space(next->parent_proc->addr_space);
  arch_cpu_write_thread_reg(next);
  arch_sched_context_switch(
      curr, next); // This function calls `sched_drop` for `curr` too.
}
