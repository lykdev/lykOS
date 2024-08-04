#pragma once

#include <core/tasking/task.h>

void sched_queue_add_task(task_t *t);

void sched_yield();

void sched_init();
