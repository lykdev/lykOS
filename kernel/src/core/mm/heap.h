#pragma once

#include <utils/def.h>

void* heap_alloc(size_t size);

void heap_free(void *obj);

void heap_init();
