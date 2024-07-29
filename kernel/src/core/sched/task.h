#pragma once

#include <lib/def.h>
#include <lib/list.h>

typedef struct task
{
    // This field is needed for architecture specific reasons.
    struct task *self;

    uptr kernel_stack;
    u64 id;

    list_node_t task_list_element;
} __attribute__((packed)) task_t;
