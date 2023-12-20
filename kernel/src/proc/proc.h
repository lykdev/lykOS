#pragma once

#include <lib/utils.h>
#include <mm/vmm.h>

enum PROC_TYPE
{
    PROC_KERNEL,
    PROC_DRIVER,
    PROC_USER
}

struct proc 
{
    // Process type: `PROC_KERNEL`, `PROC_DRIVER` or `PROC_USER`.
    u32 type;
    struct vmm_pagemap pagemap;
};