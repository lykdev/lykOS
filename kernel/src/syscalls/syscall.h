#pragma once

#include <lib/def.h>

#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/resource.h>
#include <tasking/sched.h>

typedef struct
{
    u64 value;
    u64 error;
}
syscall_result_t;

// Debug

i64  syscall_debug_log(const char *str);

// IO

i64  syscall_close(int fd);
i64  syscall_ioctl(int fd, int op, void *argp);
i64  syscall_open(const char *path, int flags, int mode);
i64  syscall_read(int fd, void *buf, u64 count);
i64  syscall_seek(int fd, u64 offset, int whence);
i64  syscall_write(int fd, void *buf, u64 count);

// Memory

i64  syscall_mmap(void *addr, u64 len, int prot, int flags, int fd, u64 off);

// Process

void syscall_exit(int code);
void syscall_tcb(void *ptr);

// Sockets

i64 syscall_accept(int sockfd, const char *addr);
i64 syscall_bind(int sockfd, const char *addr);
i64 syscall_connect(int sockfd, const char *addr);
i64 syscall_getpeername(int sockfd, char *addr);
i64 syscall_getsockname(int sockfd, char *addr);
i64 syscall_listen(int sockfd, int backlog);
i64 syscall_recv(int sockfd, void *buf, size_t len, int flags);
i64 syscall_send(int sockfd, const void *buf, size_t len, int flags);
i64 syscall_shutdown(int sockfd, int how);
i64 syscall_socket(int domain, int type, int protocol);
