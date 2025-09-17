#include "syscall.h"

#include <common/log.h>

sys_ret_t syscall_debug_log(const char *str)
{
    log("%s", str);

    return (sys_ret_t) {0, EOK};
}
