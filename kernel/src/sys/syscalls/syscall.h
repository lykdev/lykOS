#pragma once

#include <arch/cpu.h>

#include <sys/tasking.h>

static inline thread_t *syscall_get_thread()
{
    return (thread_t*)arch_cpu_read_thread_reg();
}

static inline proc_t *syscall_get_proc()
{
    return syscall_get_thread()->parent_proc;
}
