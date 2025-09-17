#include "syscall.h"

#include <common/log.h>
#include <lib/errno.h>
#include <lib/math.h>
#include <fs/vfs.h>

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

#define MAP_FAILED          ((void *)(-1))
#define MAP_ANON            0x20

sys_ret_t syscall_mmap(void *addr, u64 length, int prot, int flags, int fd, u64 offset)
{
    proc_t *proc = sched_get_curr_thread()->parent_proc;
    vmm_addr_space_t *as = proc->addr_space;

    resource_t *res;
    vnode_t *vnode;
    if (!(flags & MAP_ANON))
    {
        res = resource_get(&proc->resource_table, fd);
        if (res == NULL)
            return (sys_ret_t) {0, EBADF};
        vnode = res->node;
        if (vnode == NULL)
            return (sys_ret_t) {0, EBADF};
    }
    else
        vnode = NULL, offset = 0;

    return (sys_ret_t) {
        (u64)vmm_map_vnode(as, (uptr)addr, length, 0, VMM_MAP_ANON | VMM_MAP_PRIVATE, vnode, offset),
        EOK
    };
}
