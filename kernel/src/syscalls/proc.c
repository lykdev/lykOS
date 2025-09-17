#include "syscall.h"

#include <arch/syscall.h>
#include <common/log.h>
#include <sys/sched.h>

sys_ret_t syscall_exit(int code)
{
    log("Process exited with code: %i.", code);

    sched_yield(THREAD_STATE_AWAITING_CLEANUP);

    unreachable();
}

sys_ret_t syscall_tcb(void *ptr)
{
    arch_syscall_tcb_set(ptr);

    return (sys_ret_t) {0, EOK};
}
