#include "syscall.h"

#include <common/log.h>

void syscall_exit(int code)
{
    log("Process exited with code: %i.", code);

    while (true)
        ;
}
