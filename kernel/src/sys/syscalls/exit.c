#include "syscall.h"

#include <common/log.h>

void syscall_exit(int code)
{
    log("Process exited with code: %i.", code);

    // proc_t *proc = syscall_get_proc();

    while (true)
        ;
}
