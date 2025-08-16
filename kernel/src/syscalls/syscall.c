#include "syscall.h"

const void* syscall_table[] = {
    (void*)syscall_accept,
    (void*)syscall_bind,
    (void*)syscall_close,
    (void*)syscall_connect,
    (void*)syscall_debug_log,
    (void*)syscall_exit,
    (void*)syscall_getpeername,
    (void*)syscall_getsockname,
    (void*)syscall_ioctl,
    (void*)syscall_listen,
    (void*)syscall_mmap,
    (void*)syscall_open,
    (void*)syscall_read,
    (void*)syscall_recv,
    (void*)syscall_seek,
    (void*)syscall_send,
    (void*)syscall_shutdown,
    (void*)syscall_socket,
    (void*)syscall_tcb,
    (void*)syscall_write
};

const uptr syscall_table_length = sizeof(syscall_table) / sizeof(void*);
