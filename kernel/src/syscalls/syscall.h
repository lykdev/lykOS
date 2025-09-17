#pragma once

#include <lib/def.h>

#include <sys/proc.h>
#include <sys/resource.h>
#include <sys/sched.h>
#include <sys/thread.h>

typedef struct
{
    u64 value;
    u64 error;
}
__attribute__((packed))
sys_ret_t;

// Debug

sys_ret_t syscall_debug_log(const char *str);

// IO

sys_ret_t syscall_close(int fd);
sys_ret_t syscall_ioctl(int fd, int op, void *argp);
sys_ret_t syscall_open (const char *path, int flags, int mode);
sys_ret_t syscall_read (int fd, void *buf, u64 count);
sys_ret_t syscall_seek (int fd, u64 offset, int whence);
sys_ret_t syscall_write(int fd, void *buf, u64 count);

// Memory

sys_ret_t syscall_mmap(void *addr, u64 len, int prot, int flags, int fd, u64 off);

// Process

sys_ret_t syscall_exit(int code);
sys_ret_t syscall_tcb (void *ptr);

// Sockets

sys_ret_t syscall_accept(int sockfd, const char *addr);
sys_ret_t syscall_bind(int sockfd, const char *addr);
sys_ret_t syscall_connect(int sockfd, const char *addr);
sys_ret_t syscall_getpeername(int sockfd, char *addr);
sys_ret_t syscall_getsockname(int sockfd, char *addr);
sys_ret_t syscall_listen(int sockfd, int backlog);
sys_ret_t syscall_recv(int sockfd, void *buf, size_t len, int flags);
sys_ret_t syscall_send(int sockfd, const void *buf, size_t len, int flags);
sys_ret_t syscall_shutdown(int sockfd, int how);
sys_ret_t syscall_socket(int domain, int type, int protocol);
