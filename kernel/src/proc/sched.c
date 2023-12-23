#include "sched.h"

#include <lib/utils.h>
#include <mm/vmm.h>

extern arch_context_switch(struct task *old, struct task *new);
// Used in `arch_context_switch`.
const u64 TASK_STRUCT_RSP_OFFSET = offsetof(struct task, rsp);

static struct task task1, task2;

void test1()
{


    while (1)
    {
        for (u64 i = 0; i < 10000000; i++)
            ;
        log("TEST1");
        sched_switch(&task1, &task2);
    }
}

void test2()
{
    while(1)
    {
        for (u64 i = 0; i < 10000000; i++)
            ;
        log("TEST2");
        sched_switch(&task2, &task1);
    }
}

static void sched_task_init()
{
    // 
}

void sched_switch(struct task *old, struct task *new)
{
    vmm_switch_to_map(new->pagemap);

        arch_context_switch(old, new);
}

#if defined (__x86_64__)

#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/lapic.h>

struct init_stack_kernel
{
    u64 r15, r14, r13, r12, rbp, rbx;
    void (* thread_init)(task *prev);
    void (* entry)();
} __attribute__((packed));

#endif

#include <mm/pmm.h>

void sched_init()
{
    task1.rsp = (u64)pmm_alloc() + PAGE_SIZE;
    task2.rsp = (u64)pmm_alloc() + PAGE_SIZE;

    struct init_stack_kernel *s = task2.rsp - sizeof(struct init_stack_kernel);
    task2.rsp = (u64)s;

    s->entry = test2;
    s->thread_init = sched_task_init;
}