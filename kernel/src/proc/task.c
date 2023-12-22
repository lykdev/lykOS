#include "task.h"

#include <mm/pmm.h>

#if defined (__x86_64__)

void task_setup(struct task *t, uptr entry_addr)
{
    t->regs.rip = entry_addr;
}
#endif