#pragma once

#include <core/mm/vmm.h>

#include <utils/def.h>
#include <utils/list.h>

typedef struct cpu_core cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

struct cpu_core
{
    struct cpu_core_t *self;
    u64 id;
    thread_t *curr_thread;
    thread_t *idle_thread;
    list_node_t list_elem;
};

struct proc
{
    u64 id;
    char name[64];
    vmm_addr_space_t *addr_space;
    list_t threads;
    list_node_t list_elem;
};

struct thread
{
    u64 id;
    proc_t *parent_proc;
    cpu_core_t *assigned_core;
    void *stack;

    __attribute__((aligned(8)))
    list_node_t list_elem_thread;
    list_node_t list_elem_inside_proc;
}
__attribute__((packed));
