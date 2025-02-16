#include <utils/log.h>

void syscall_exit()
{
    log("EXIT");
    while (true)
        ;    
}
