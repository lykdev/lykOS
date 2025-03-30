#pragma once

#include <arch/cpu.h>

#include <sys/proc.h>
#include <sys/thread.h>

static inline thread_t *syscall_get_thread()
{
    return (thread_t *)arch_cpu_read_thread_reg();
}

static inline proc_t *syscall_get_proc()
{
    return syscall_get_thread()->parent_proc;
}

void  syscall_exit(int code);
u64   syscall_debug_log(const char *str);
int   syscall_open(const char *path, int flags, int mode);
int   syscall_close(int fd);
int   syscall_read(int fd, void *buf, u64 count);
int   syscall_write(int fd, void *buf, u64 count);
void *syscall_mmap(void *addr, u64 len, int prot, int flags, int fd, u64 off);

static void* g_syscall_table[] = {
    (void*)syscall_exit,
    (void*)syscall_debug_log,
    (void*)syscall_open,
    (void*)syscall_close,
    (void*)syscall_read,
    (void*)syscall_write,
    (void*)syscall_mmap
};
