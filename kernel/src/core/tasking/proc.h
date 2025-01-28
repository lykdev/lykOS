#pragma once

#include <core/tasking/def.h>

proc_t *proc_new(char *name, proc_privilege_t priv);

proc_t* proc_find_id(u64 id);
