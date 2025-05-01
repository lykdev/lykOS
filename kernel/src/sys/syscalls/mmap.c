#include "syscall.h"

#include <common/log.h>
#include <lib/math.h>

// Prot
#define PROT_NONE  0
#define PROT_READ  (1 << 0)
#define PROT_WRITE (1 << 1)
#define PROT_EXEC  (1 << 2)

// Flags
#define MAP_ANON      (1 << 0)
#define MAP_ANONYMOUS (1 << 1)
#define MAP_FIXED     (1 << 2)

// Return
#define MAP_FAILED ((void*)(-1))

void *syscall_mmap(void *addr, u64 len, int prot, int flags, int fd, u64 off)
{
    log("MMAP");
    proc_t *proc = syscall_get_proc();
    vmm_addr_space_t *as = proc->addr_space;

    len = CEIL(len, 4096);

    uptr _addr = vmm_find_space(as, len);

    prot &= 0b111; // Sanitize protection flags.

    if (flags & MAP_ANON)
        vmm_map_anon(as, _addr, len, prot);
    else
        return MAP_FAILED;

    return (void*)_addr;
}
