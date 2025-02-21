#include <arch/cpu.h>

#include <graphics/video.h>
#include <mm/vmm.h>
#include <sys/tasking.h>

#include <utils/math.h>
#include <utils/hhdm.h>
#include <utils/log.h>

u64 syscall_malloc(u64 size)
{
    log("MALLOC");
    proc_t *proc = ((thread_t*)arch_cpu_read_thread_reg())->parent_proc;
    
    size = CEIL(size, 0x1000);
    uptr ret = vmm_find_space(proc->addr_space, size);

    vmm_map_anon(proc->addr_space, ret, size, VMM_FULL);

    return ret;
}
