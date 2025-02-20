#include <arch/cpu.h>

#include <graphics/video.h>
#include <mm/vmm.h>
#include <sys/tasking.h>

#include <utils/math.h>
#include <utils/hhdm.h>
#include <utils/log.h>

void syscall_map_fb(void *fb, uint *w, uint *h, uint *pitch)
{
    log("MAP_FB");
    proc_t *proc = ((thread_t*)arch_cpu_read_thread_reg())->parent_proc;
    u64     size =  CEIL(video_fb.height * video_fb.pitch * 4, ARCH_PAGE_GRAN);
    vmm_map_direct(proc->addr_space, (uptr)fb, size, VMM_FULL, FLOOR(video_fb.addr - HHDM, ARCH_PAGE_GRAN));
    *w = video_fb.width;
    *h = video_fb.height;
    *pitch = video_fb.pitch;
}
