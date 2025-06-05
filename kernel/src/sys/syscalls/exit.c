#include "syscall.h"

#include <arch/cpu.h>
#include <common/log.h>

void syscall_exit(int code)
{
    log("Process exited with code: %i.", code);

    arch_cpu_int_unmask();
    while (true)
        ;
}
