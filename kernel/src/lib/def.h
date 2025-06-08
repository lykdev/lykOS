#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef uintptr_t uptr;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef unsigned int uint;

#define KIB 1024ull
#define MIB (KIB * 1024ull)
#define GIB (MIB * 1024ull)

#define BAD_ADDRESS ((uptr)-1)
