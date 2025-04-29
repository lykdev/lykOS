#pragma once

#include <lib/def.h>

extern u32 g_x86_64_fpu_area_size;
extern void (*g_x86_64_fpu_save)(void *area);
extern void (*g_x86_64_fpu_restore)(void *area);

void x86_64_fpu_init();

void x86_64_fpu_init_cpu();
