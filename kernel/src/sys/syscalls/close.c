#include "syscall.h"

#include <fs/vfs.h>
#include <common/log.h>

int syscall_close(int fd)
{
    log("CLOSE");
    proc_t *proc = syscall_get_proc();
    
    proc->resource_table[fd] = NULL;

    return 0;
}
