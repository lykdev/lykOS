#include <utils/log.h>

void syscall_exit(u64 code)
{
    log("EXIT");
    while (true)
        ;    
}
