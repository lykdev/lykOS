#pragma once

#include <lib/def.h>
#include <lib/list.h>

typedef struct task
{
    /**
     * The position of 'self' and 'kernel_stack' fields within the struct must not be changed,
     * as they are needed for architecture-specific reasons.
     * 
     * 'self' is needed for x86_64 optimizations.
     */

    /// @brief Pointer to the parent structure.
    struct task *self;
    /// @brief Kernel stack for the task.
    uptr kernel_stack;

    /// @brief Task's unique identifier.
    u64 id;
    /// @brief Linked list element for the task list.
    list_node_t task_list_element;

} __attribute__((packed)) task_t;
