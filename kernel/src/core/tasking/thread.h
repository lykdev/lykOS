#pragma once

#include <core/tasking/def.h>

thread_t *thread_new(proc_t *parent_proc, void *entry);
