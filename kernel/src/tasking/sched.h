#pragma once

#include <sys/thread.h>

thread_t *sched_get_curr_thread();

void sched_drop(thread_t *thread);

void sched_queue_add(thread_t *thread);

void sched_yield(thread_status_t status);

void sched_init();
