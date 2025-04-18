#pragma once

#include <common/limine/limine.h>

extern volatile struct limine_framebuffer_request request_framebuffer;

extern volatile struct limine_hhdm_request request_hhdm;

extern volatile struct limine_executable_address_request request_kernel_addr;

extern volatile struct limine_memmap_request request_memmap;

extern volatile struct limine_module_request request_module;

extern volatile struct limine_mp_request request_mp;

extern volatile struct limine_rsdp_request request_rsdp;
