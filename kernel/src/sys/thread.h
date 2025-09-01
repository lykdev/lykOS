#pragma once

#include <arch/thread.h>
#include <lib/def.h>
#include <lib/list.h>
#include <sys/proc.h>

typedef enum proc_type proc_type_t;
typedef struct smp_cpu_core smp_cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

typedef int tid_t;

typedef enum
{
    THREAD_STATE_AWAITING_CLEANUP,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING
}
thread_status_t;

struct thread
{
    arch_thread_context_t context;
    tid_t id;
    thread_status_t status;
    proc_t *parent_proc;
    smp_cpu_core_t *assigned_core;

    spinlock_t slock;
    list_node_t list_node_sched;
    list_node_t list_node_proc;
    list_node_t list_node_mutex;
    /*
     * `ref_count` starts at 0 but will increment if:
     * - it is scheduled
     * - it is assigned to a parent process
     * - it is enqueued inside a mutex
     */
    u64 ref_count;
};

extern u64 g_thread_count;

inline void thread_ref(thread_t *t)
{
    spinlock_acquire(&t->slock);

    t->ref_count++;

    spinlock_release(&t->slock);
}

inline void thread_unref(thread_t *t)
{
    spinlock_acquire(&t->slock);

    t->ref_count--;

    spinlock_release(&t->slock);
}

void thread_new(proc_t *parent_proc, uptr entry);
