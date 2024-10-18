#pragma once

#include <utils/def.h>

#define log(FORMAT, ...) _log(__FILE__, FORMAT, ##__VA_ARGS__)

void _log(const char *file, const char *format, ...);

void _n_log(const char *file, const char *format, va_list list);
