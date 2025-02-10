#pragma once

#include <utils/def.h>

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
    #error Unimplemented.
#endif
    void (*entry)();
}
__attribute__((packed))
arch_thread_init_stack_t;