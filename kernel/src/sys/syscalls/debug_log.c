#include <common/log.h>

void syscall_debug_log(const char *str)
{
    log("MLIBC: %s", str);
}
