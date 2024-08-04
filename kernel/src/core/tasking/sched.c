#include "sched.h"

#include <arch/cpu.h>
#include <arch/int.h>

#include <core/mm/pmm.h>
#include <core/tasking/task.h>

#include <lib/def.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/list.h>
#include <lib/slock.h>
#include <lib/hhdm.h>
#include <lib/mem.h>

extern void arch_context_switch(task_t *curr, task_t *next) __attribute__((naked));

typedef struct
{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rbx;
    u64 rdx;
    u64 rcx;
    u64 rax;
    uptr init_func;
    uptr entry;
} __attribute__((packed)) task_entry_stack_t;

list_t  task_list = LIST_INIT;
slock_t task_list_lock = SLOCK_INIT;

task_t t1, t2, t3;

void f1()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 1 - %d", i);
        sched_yield();
    }
}

void f2()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 2 - %d", i);
        sched_yield();
    }
}

void f3()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 3 - %d", i);
        sched_yield();
    }
}

void sched_queue_add_task(task_t *t)
{
    slock_acquire(&task_list_lock);

    list_append(&task_list, &t->task_list_element);

    slock_release(&task_list_lock);
}

static void task_init_func()
{
    int_enable();
}

static u64 _id = 0;

void task_init(task_t *t, uptr entry)
{
    t->self = t;
    t->id = _id++;

    uptr stack_region = pmm_alloc(3)->phys_addr + HHDM; // Allocate 8 pages (32KiB)
    memset((void*)stack_region, 0, 32 * KIB);
    
    task_entry_stack_t *frame = (task_entry_stack_t*)(stack_region + 32 * KIB - sizeof(task_entry_stack_t));
    frame->entry = entry;
    frame->init_func = (uptr)&task_init_func;

    t->kernel_stack = (uptr)frame;

    sched_queue_add_task(t);
}

task_t* sched_next_task()
{
    slock_acquire(&task_list_lock);

    list_node_t *node = LIST_FIRST(&task_list);
    list_remove(&task_list, node);

    slock_release(&task_list_lock);

    return LIST_GET_CONTAINER(node, task_t, task_list_element);
}

void sched_yield()
{
    int_disable();

    task_t *curr = cpu_lcore_get_current_task();
    ASSERT(curr != NULL);
    task_t *next = sched_next_task();
    ASSERT(next != NULL);

    cpu_lcore_set_current_task(next);
    arch_context_switch(curr, next); // This also adds the old task back to the queue.

    int_enable();
}

void sched_init()
{
    task_init(&t1, (uptr)f1);
    task_init(&t2, (uptr)f2);
    task_init(&t3, (uptr)f3);

    cpu_lcore_set_current_task(&t1);
    f1();
}
