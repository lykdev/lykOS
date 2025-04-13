#pragma once

#include <sys/proc.h>
#include <lib/def.h>
#include <lib/list.h>

typedef enum proc_type proc_type_t;
typedef struct smp_cpu_core smp_cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

struct thread
{
// DO NOT TOUCH THIS PART
#if defined(__x86_64__)
    thread_t *self;
#endif
    uptr kernel_stack;
    uptr syscall_stack;
//
    uint id;
    proc_t *parent_proc;
    smp_cpu_core_t *assigned_core;

    list_node_t list_elem_thread;
    list_node_t list_elem_inside_proc;
};

extern list_t g_thread_list;

thread_t *thread_new(proc_t *parent_proc, uptr entry);
