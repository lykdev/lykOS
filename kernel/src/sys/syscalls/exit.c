#include "syscall.h"

#include <utils/log.h>

void syscall_exit()
{
    log("EXIT");

    proc_t *proc = syscall_get_proc();

    while (true)
        ;    
}
