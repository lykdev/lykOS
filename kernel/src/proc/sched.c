#include "sched.h"

#include <lib/utils.h>

struct task *curr_thread;

#if defined (__x86_64__)

#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/lapic.h>

void sched_jump_to_thread(struct task *t)
{
    void (*jump_addr)() = t->regs.rip;
    jump_addr();
}

char a[16] = "TEST!";

void sched_next(struct int_regs *regs)
{
    log("TEST3");
    lapic_timer_oneshoot(32, 1000 * 1000);
}

void sched_init()
{
    irq_register_handler(0, &sched_next);
    lapic_timer_oneshoot(32, 1000 * 1000);
}

#endif