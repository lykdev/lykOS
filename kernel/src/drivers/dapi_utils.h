#pragma once

#include <stdarg.h>
#include <stddef.h>

typedef struct dapi_utils dapi_utils_t;

struct dapi_utils {
  bool (*log)(const char *format, ...);
  void *(*alloc)(size_t size);
  void (*free)(void *ptr);
};
