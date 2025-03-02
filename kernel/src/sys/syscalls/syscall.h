#pragma once

#include <arch/cpu.h>

#include <sys/tasking.h>
#include <sys/resource.h>

static inline thread_t *syscall_get_thread()
{
    return (thread_t *)arch_cpu_read_thread_reg();
}

static inline proc_t *syscall_get_proc()
{
    return syscall_get_thread()->parent_proc;
}

void syscall_exit();
u64  syscall_debug_log(const char *str);
int  syscall_open(const char *path, int flags, int mode);
int  syscall_close(int fd);
int  syscall_write(int fd, u64 count, void *buf);
int  syscall_read(int fd, u64 count, void *buf);

static void* g_syscall_table[] __attribute__((packed)) = {
    (void*)syscall_exit,
    (void*)syscall_debug_log,
    (void*)syscall_open,
    (void*)syscall_close,
    (void*)syscall_write,
    (void*)syscall_read
};
