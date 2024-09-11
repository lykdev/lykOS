#pragma once

#include <utils/def.h>

#define log(LEVEL, FORMAT, ...) _log("my_module", LEVEL, FORMAT, ##__VA_ARGS__)

void _log(const char *module, int level, const char *format, ...);
