#pragma once

#include <lib/utils.h>
#include <mm/vmm.h>

struct thread_regs
{
    u64 rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip;
};

struct thread
{
    // Pointer to the pagemap of the parent process.
    struct vmm_pagemap *pagemap;
    // CPU task-specific registers.
    struct thread_regs regs;
};