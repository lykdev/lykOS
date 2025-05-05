#include "syscall.h"

#include <fs/vfs.h>
#include <common/log.h>

int syscall_close(int fd)
{
    proc_t *proc = syscall_get_proc();

    proc->resource_table.resources[fd] = NULL;

    return 0;
}
