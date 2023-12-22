#pragma once

#include <proc/task.h>

void sched_init();

void sched_jump_to_thread(struct task *t);
