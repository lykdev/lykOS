#pragma once

#include <sys/thread.h>
#include <lib/def.h>
#include <lib/list.h>

typedef enum proc_type proc_type_t;
typedef struct smp_cpu_core smp_cpu_core_t;
typedef struct thread thread_t;
typedef struct proc proc_t;

struct smp_cpu_core
{
    uint id;
    thread_t *curr_thread;
    thread_t *idle_thread;
    list_node_t list_elem;
};

extern list_t g_smp_cpu_core_list;

extern bool g_smp_initialized;

void smp_init();
