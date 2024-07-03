#pragma once

#include <lib/def.h>
#include <lib/list.h>

typedef struct task
{
    struct task *self;

    u64 id;
    uptr kernel_stack;

    list_node_t *task_list_element;
} __attribute__((packed)) task_t;
