#pragma once

#include <lib/def.h>

void _log(u32 color, const char *format, va_list list);

void log(const char *format, ...);

void log_warn(const char *format, ...);

void log_err(const char *format, ...);