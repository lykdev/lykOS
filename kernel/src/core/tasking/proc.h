#pragma once

#include <core/tasking/def.h>

proc_t *proc_new(char *name, bool privileged);

proc_t* proc_find_id(u64 id);
