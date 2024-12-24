#pragma once

#include <core/mm/vmm.h>

#include <utils/def.h>
#include <utils/list.h>

typedef struct cpu_core cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

struct cpu_core
{
    u64 id;
    thread_t *curr_thread;
    thread_t *idle_thread;
    list_node_t list_elem;
};

struct thread
{
#if defined (__x86_64__)
    struct thread_t *self;
#endif
    u64 id;
    proc_t *parent_proc;
    cpu_core_t *assigned_core;
    void *stack;

    __attribute__((aligned(8)))
    list_node_t list_elem_thread;
    list_node_t list_elem_inside_proc;
} __attribute__((packed));

struct proc
{
    u64 id;
    char name[64];
    vmm_addr_space_t addr_space;
    list_t threads;
    list_node_t list_elem;
};

extern list_t sched_cpu_core_list;
extern list_t sched_thread_list;
extern list_t sched_proc_list;

proc_t *proc_new(char *name, bool privileged);

proc_t* proc_find_id(u64 id);

thread_t *thread_new(proc_t *parent_proc, void *entry);

void sched_yield();

void sched_init();
