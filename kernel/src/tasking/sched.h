#pragma once

#include <sys/thread.h>

thread_t *sched_get_curr_thread() __attribute__((always_inline));

void sched_drop(thread_t *thread);

void sched_queue_add(thread_t *thread);

void sched_yield(thread_status_t status);
