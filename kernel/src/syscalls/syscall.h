#pragma once

#include <arch/cpu.h>
#include <lib/def.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <tasking/sched.h>

int   syscall_close(int fd);
u64   syscall_debug_log(const char *str);
void  syscall_exit(int code);
int   syscall_seek(int fd, u64 offset, int whence);
void *syscall_mmap(void *addr, u64 len, int prot, int flags, int fd, u64 off);
int   syscall_open(const char *path, int flags, int mode);
int   syscall_read(int fd, void *buf, u64 count);
int   syscall_write(int fd, void *buf, u64 count);
void  syscall_tcb(void *ptr);

// static void* g_syscall_table[] = {
//     (void*)syscall_close,
//     (void*)syscall_debug_log,
//     (void*)syscall_exit,
//     (void*)syscall_seek,
//     (void*)syscall_mmap,
//     (void*)syscall_open,
//     (void*)syscall_read,
//     (void*)syscall_write,
//     (void*)syscall_tcb,
// };
