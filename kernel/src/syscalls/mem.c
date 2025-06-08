#include "syscall.h"

#include <common/log.h>
#include <lib/math.h>
#include <fs/vfs.h>

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

#define MAP_FAILED          ((void *)(-1))
// #define MAP_FILE            0x00
// #define MAP_PRIVATE         0x01
// #define MAP_SHARED          0x02
// #define MAP_FIXED           0x04
// #define MAP_ANON            0x08
// #define MAP_ANONYMOUS       0x08
// #define MAP_NORESERVE       0x10
// #define MAP_FIXED_NOREPLACE 0x20

#define MAP_ANON      (1 << 0)
#define MAP_ANONYMOUS (1 << 1)
#define MAP_FIXED     (1 << 2)

#define MS_ASYNC        0x01
#define MS_SYNC         0x02
#define MS_INVALIDATE   0x04

#define MCL_CURRENT 0x01
#define MCL_FUTURE  0x02

#define POSIX_MADV_NORMAL     1
#define POSIX_MADV_SEQUENTIAL 2
#define POSIX_MADV_RANDOM     3
#define POSIX_MADV_DONTNEED   4
#define POSIX_MADV_WILLNEED   5

#define MADV_NORMAL     0
#define MADV_RANDOM     1
#define MADV_SEQUENTIAL 2
#define MADV_WILLNEED   3
#define MADV_DONTNEED   4
#define MADV_FREE       8

void *syscall_mmap(void *addr, u64 length, int prot, int flags, int fd, u64 offset)
{
    proc_t *proc = syscall_get_proc();
    vmm_addr_space_t *as = proc->addr_space;

    length = CEIL(length, ARCH_PAGE_GRAN);

    uptr _addr = vmm_find_space(as, length);
    if ((flags & MAP_FIXED) && _addr != (uptr)addr)
        return MAP_FAILED;

    if (flags & MAP_ANON)
    {
        vmm_map_anon(as, _addr, length, prot);
        vmm_zero_out(as, _addr, length);
    }
    else
    {
        // do fd stuff
        return MAP_FAILED;
    }

    return (void*)_addr;
}
