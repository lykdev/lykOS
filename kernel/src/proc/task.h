#pragma once

#include <lib/utils.h>
#include <mm/vmm.h>

struct task_regs
{
    #if defined (__x86_64__)
        u64 rax, rbx, rcx, rdx, rsi, rdi;
        u64 r8, r9, r10, r11, r12, r13, r14, r15;
        u64 rsp, rbp;
        u64 rip;        
    #endif
};

struct task
{
    // Pointer to the pagemap of the parent process.
    struct vmm_pagemap *pagemap;
    // CPU task-specific registers.
    struct task_regs regs;
};

void task_setup(struct task *t, uptr entry_addr);