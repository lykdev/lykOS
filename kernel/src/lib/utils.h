#pragma once

#include "limine.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#define KIB 1024ULL
#define MIB (KIB * 1024ULL)
#define GIB (MIB * 1024ULL)

#define PAGE_SIZE 0x1000ULL

extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_kernel_address_request kernel_addr_request;
extern volatile struct limine_rsdp_request rsdp_request;
extern volatile struct limine_smp_request smp_request;

#define HHDM (hhdm_request.response->offset)
#define KERNEL_ADDR_VIRT (kernel_addr_request.response->virtual_base)
#define KERNEL_ADDR_PHYS (kernel_addr_request.response->physical_base)

#define RGB(r, g, b) (r << 16) | (g << 8) | b

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t uptr;

void halt();

#define ASSERT(CONDITION) if (!(CONDITION)) panic("Assertion false. \"%s\" at %s:%d", #CONDITION, __FILE__, __LINE__);
#define ASSERT_C(CONDITION, COMMENT) if (!(CONDITION)) panic("%s - \"%s\" at %s:%d", #COMMENT, #CONDITION, __FILE__, __LINE__);

void panic(const char *format, ...);

void log(const char *format, ...);

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *s, int c, size_t n);

void *memmove(void *dest, const void *src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);
