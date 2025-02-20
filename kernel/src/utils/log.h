#pragma once

#include <utils/def.h>

#define log(FORMAT, ...) _log(FORMAT, ##__VA_ARGS__)

void _log(const char *format, ...);

void _n_log(const char *format, va_list list);
