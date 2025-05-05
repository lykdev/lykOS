#include "syscall.h"

#include <arch/syscall.h>
#include <common/log.h>

void syscall_tcb(void *ptr)
{
    arch_syscall_tcb_set(ptr);
}
