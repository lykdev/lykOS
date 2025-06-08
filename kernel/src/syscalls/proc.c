#include "syscall.h"

#include <arch/syscall.h>
#include <common/log.h>
#include <tasking/sched.h>

void syscall_exit(int code)
{
    log("Process exited with code: %i.", code);

    sched_yield(THREAD_STATE_AWAITING_CLEANUP);
}

void syscall_tcb(void *ptr)
{
    arch_syscall_tcb_set(ptr);
}
