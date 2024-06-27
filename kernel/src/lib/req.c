#include "req.h"

volatile struct limine_framebuffer_request req_framebuffer = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

volatile struct limine_hhdm_request req_hhdm = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

volatile struct limine_kernel_address_request req_kernel_addr = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

volatile struct limine_memmap_request req_memmap = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

volatile struct limine_rsdp_request req_rsdp = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

volatile struct limine_smp_request req_smp = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};
