#pragma once

#include <lib/utils.h>
#include <mm/vmm.h>

enum
{
    TASK_KERNEL,
    TASK_USER
};

struct task
{
    struct task *self;
    // `TASK_KERNEL` or `TASK_USER`.
    u8 type;
    // Pointer to the pagemap of the parent process.
    struct vmm_pagemap *pagemap;
    u64 rsp;
} __attribute__((packed));

void task_setup(struct task *t, uptr entry_addr);