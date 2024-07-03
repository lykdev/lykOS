#include <core/sched/sched.h>

#include <arch/cpu.h>

#include <core/sched/task.h>

#include <lib/def.h>
#include <lib/log.h>
#include <lib/list.h>
#include <lib/slock.h>

extern void arch_context_switch(task_t *curr, task_t *next) __attribute__((naked));

list_t  task_list;
slock_t task_list_lock = SLOCK_INIT;

task_t t1, t2, t3, t4;

void f1()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 1");
        sched_yield();
    }
}

void f2()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 2");
        sched_yield();
    }
        
}

void f3()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 3");
        sched_yield();
    }
        
}

void f4()
{
    for (u64 i = 0; i < 5; i++)
    {
        log("TASK 4");
        sched_yield();
    }        
}

static u64 i = 0;

void task_init(task_t *t, uptr entry)
{
    t->self = t;

    t->id = i++;

    list_append(&task_list, t->task_list_element);
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
    task_t *curr = cpu_get_current_task();
    task_t *next = sched_next_task();

    log("curr: %u, next %u", curr->id, next->id);

    arch_context_switch(curr, next);
}

void sched_init()
{
    task_init(&t1, (uptr)f1);
    task_init(&t2, (uptr)f2);
    task_init(&t3, (uptr)f3);
    task_init(&t4, (uptr)f4);

    __asm__ volatile ("jmp *%0\n" :: "r"(&f1));
}