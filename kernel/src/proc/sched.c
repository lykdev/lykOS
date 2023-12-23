#include "sched.h"

#include <lib/utils.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#if defined (__x86_64__)

#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/lapic.h>
#include <arch/x86_64/msr.h>

struct init_stack_kernel
{
    u64 r15, r14, r13, r12, rbp, rbx;
    void (* entry)();
} __attribute__((packed));

#endif

struct task *arch_sched_thread_current()
{
    struct task *t = 0;
    asm volatile("mov %%gs:0, %0" : "=r" (t));
    return &t;
}

extern struct task* arch_context_switch(struct task *old, struct task *new);

static struct task task1, task2;

void sched_switch(struct task *old, struct task *new)
{
#if defined (__x86_64__)
    msr_write(MSR_GS_BASE, (u64)new); // Set the current task *register*.
#endif

    vmm_switch_to_map(new->pagemap);

    arch_context_switch(old, new);
}

void sched_timer_handler(struct int_regs *regs)
{
    log("fsdfsghj");
    // lapic_timer_oneshoot(32, 10000000);

    if (arch_sched_thread_current() == &task1)
        sched_switch(arch_sched_thread_current(), &task2);
    else
        sched_switch(arch_sched_thread_current(), &task1);
}

void test1()
{
    while (1)
    {
        for (u64 i = 0; i < 10000000; i++)
            ;
        log("TEST1");
    }
}

void test2()
{
    while(1)
    {
        for (u64 i = 0; i < 10000000; i++)
            ;
        log("TEST2");
    }
}


void sched_init()
{
    task1.pagemap = &kernelmap;
    task1.self = &task1;

    task2.pagemap = &kernelmap;
    task2.self = &task2;
    task2.rsp = (u64)pmm_alloc() + PAGE_SIZE + HHDM;
    memset((void*)((uptr)task2.rsp - PAGE_SIZE), 0, PAGE_SIZE);

    struct init_stack_kernel *s = (struct init_stack_kernel*)(task2.rsp - sizeof(struct init_stack_kernel));
    task2.rsp = (u64)s;

    s->entry = test2;

    irq_register_handler(0, &sched_timer_handler);
    lapic_timer_oneshoot(32, 10000000);

    msr_write(MSR_GS_BASE, (u64)&task1);
    test1();
}