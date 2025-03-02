#include "syscall.h"

#include <fs/vfs.h>
#include <common/log.h>

int syscall_open(const char *path, int flags, int mode)
{
    log("OPEN");
    proc_t *proc = syscall_get_proc();
    
    vfs_node_t *node;
    if(vfs_lookup(path, &node) < 0)
    {
        log("FAILED");
    }

    return resource_create(&proc->resource_table, node, 0, RESOURCE_READ | RESOURCE_WRITE);
}
