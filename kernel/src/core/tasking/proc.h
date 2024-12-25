#pragma once

#include <core/tasking/def.h>

#define PROC_KERNEL 1 << 0

proc_t *proc_new(char *name, uint flags);

proc_t* proc_find_id(u64 id);
