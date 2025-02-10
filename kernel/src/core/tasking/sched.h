#pragma once

#include <core/tasking/tasking.h>

void sched_drop(thread_t *thread);

void sched_thread_init();

void sched_queue_add(thread_t *thread);

void sched_yield();
