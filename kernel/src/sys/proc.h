#pragma once

#include <lib/def.h>
#include <lib/list.h>
#include <mm/vmm.h>
#include <sys/resource.h>

typedef enum proc_type proc_type_t;
typedef struct cpu_core cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

typedef int pid_t;

enum proc_type
{
    PROC_KERNEL,
    PROC_USER
};

struct proc
{
    pid_t id;
    proc_type_t type;
    char name[64];
    vmm_addr_space_t *addr_space;
    list_t threads;
    resource_table_t resource_table;

    spinlock_t slock;
    list_node_t list_node;
    u64 ref_count;
};

extern list_t g_proc_list;

inline void proc_ref(proc_t *p)
{
    spinlock_acquire(&p->slock);

    p->ref_count++;

    spinlock_release(&p->slock);
}

inline void proc_unref(proc_t *p)
{
    spinlock_acquire(&p->slock);

    p->ref_count--;

    spinlock_release(&p->slock);
}

proc_t *proc_new(proc_type_t type);

proc_t *proc_find_id(pid_t id);
