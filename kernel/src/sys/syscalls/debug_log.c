#include <common/log.h>

void syscall_debug_log(const char *str)
{
    log("DEBUG LOG: %s", str);
}
