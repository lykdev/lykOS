#pragma once

#include <core/mm/vmm.h>

#include <utils/def.h>
#include <utils/list.h>

typedef struct cpu_core cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

struct cpu_core
{
    uint id;
    thread_t *curr_thread;
    thread_t *idle_thread;
    list_node_t list_elem;
};

typedef enum
{
    PROC_KERNEL,
    PROC_USER
} proc_privilege_t;

struct proc
{
    uint id;
    char name[64];
    proc_privilege_t priv;
    vmm_addr_space_t *addr_space;
    list_t threads;
    list_node_t list_elem;
};

struct thread
{
// DO NOT TOUCH THIS PART
#if defined (__x86_64__)
    thread_t *self;
#endif
    void *stack;
//
    uint id;
    proc_t *parent_proc;
    cpu_core_t *assigned_core;

    __attribute__((aligned(8)))
    list_node_t list_elem_thread;
    list_node_t list_elem_inside_proc;
}
__attribute__((packed));

extern list_t g_cpu_core_list;
extern list_t g_thread_list;
extern list_t g_proc_list;

extern bool g_smp_initialized;