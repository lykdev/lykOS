#pragma once

#include <lib/def.h>

uptr ksym_resolve_symbol(const char *name);

void ksym_load_symbols();
