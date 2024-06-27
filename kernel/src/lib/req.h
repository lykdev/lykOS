#pragma once

#include <limine.h>

extern volatile struct limine_framebuffer_request req_framebuffer;

extern volatile struct limine_hhdm_request req_hhdm;

extern volatile struct limine_kernel_address_request req_kernel_addr;

extern volatile struct limine_memmap_request req_memmap;

extern volatile struct limine_rsdp_request req_rsdp;

extern volatile struct limine_smp_request req_smp;
