#pragma once

#define LIKELY(V) (__builtin_expect(!!(V), 1))
#define UNLIKELY(V) (__builtin_expect(!!(V), 0))
