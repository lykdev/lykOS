#pragma once

#include <core/tasking/def.h>

extern list_t sched_cpu_core_list;
extern list_t sched_thread_list;
extern list_t sched_proc_list;

void sched_yield();

void sched_init();
