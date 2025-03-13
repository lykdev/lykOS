#pragma once

#include <sys/thread.h>

void sched_drop(thread_t *thread);

void sched_queue_add(thread_t *thread);

void sched_yield();
