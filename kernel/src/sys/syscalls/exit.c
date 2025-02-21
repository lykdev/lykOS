#include <utils/log.h>

u64 syscall_exit()
{
    log("EXIT");
    while (true)
        ;    
}
