#pragma once

// Read on Segment Selectors.

#define X86_64_GDT_SEL_CODE_RING0 (1 << 3)
#define X86_64_GDT_SEL_DATA_RING0 (2 << 3)
#define X86_64_GDT_SEL_DATA_RING3 ((3 << 3) | 0b11)
#define X86_64_GDT_SEL_CODE_RING3 ((4 << 3) | 0b11)

void x86_64_gdt_load();
