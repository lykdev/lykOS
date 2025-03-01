#pragma once

#include <lib/def.h>

typedef struct
{
#if defined (__x86_64__)
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;
#elif defined (__aarch64__)
    u64 x30;
    u64 x29;
    u64 x28;
    u64 x27;
    u64 x26;
    u64 x25;
    u64 x24;
    u64 x23;
    u64 x22;
    u64 x21;
    u64 x20;
    u64 x19;
    u64 x18;
    u64 x17;
    u64 x16;
    u64 x15;
    u64 x14;
    u64 x13;
    u64 x12;
    u64 x11;
    u64 x10;
    u64 x9;
    u64 x8;
    u64 x7;
    u64 x6;
    u64 x5;
    u64 x4;
    u64 x3;
    u64 x2;
    u64 x1;
    u64 x0;
#endif
    uptr entry;
}
__attribute__((packed))
arch_thread_init_stack_kernel_t;

typedef struct
{
#if defined (__x86_64__)
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;
#elif defined (__aarch64__)
    u64 x30;
    u64 x29;
    u64 x28;
    u64 x27;
    u64 x26;
    u64 x25;
    u64 x24;
    u64 x23;
    u64 x22;
    u64 x21;
    u64 x20;
    u64 x19;
    u64 x18;
    u64 x17;
    u64 x16;
    u64 x15;
    u64 x14;
    u64 x13;
    u64 x12;
    u64 x11;
    u64 x10;
    u64 x9;
    u64 x8;
    u64 x7;
    u64 x6;
    u64 x5;
    u64 x4;
    u64 x3;
    u64 x2;
    u64 x1;
    u64 x0;
#endif
    void (*userspace_init)();
    uptr entry;
    u64  user_stack;
}
__attribute__((packed))
arch_thread_init_stack_user_t;
