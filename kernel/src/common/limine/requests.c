#include "requests.h"
#include "common/limine/limine.h"

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests_start_"
                             "marker"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

// The actual requests.

__attribute__((used, section(".requests"))) volatile struct limine_framebuffer_request request_framebuffer = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used, section(".requests"))) volatile struct limine_hhdm_request request_hhdm = {.id = LIMINE_HHDM_REQUEST, .revision = 0};

__attribute__((used, section(".requests"))) volatile struct limine_executable_address_request request_kernel_addr = {.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST, .revision = 0};

__attribute__((used, section(".requests"))) volatile struct limine_memmap_request request_memmap = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};

__attribute__((used, section(".requests"))) volatile struct limine_module_request request_module = {.id = LIMINE_MODULE_REQUEST, .revision = 0};

__attribute__((used, section(".requests"))) volatile struct limine_mp_request request_mp = {.id = LIMINE_MP_REQUEST, .revision = 0};

__attribute__((used, section(".requests"))) volatile struct limine_rsdp_request request_rsdp = {.id = LIMINE_RSDP_REQUEST, .revision = 0};
