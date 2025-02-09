#include <utils/log.h>

u64 syscall_debug_log(const char *str, u64 len)
{
    if (len > 1024)
        len = 1024;
    log("DEBUG LOG: %.*s", len, str);

    return 0;
}
