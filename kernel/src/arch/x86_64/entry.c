#include <common/log.h>
#include <common/panic.h>
#include <graphics/video.h>
#include <graphics/draw.h>

#include <arch/cpu.h>

#include <arch/x86_64/tables/gdt.h>
#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/devices/pic.h>
#include <arch/x86_64/devices/ioapic.h>
#include <arch/x86_64/fpu.h>

#include <dev/acpi/acpi.h>

[[noreturn]] extern void kernel_main();

void _entry()
{
    arch_cpu_int_mask();

    video_init();
    log("Video: %ux%ux%u - pitch %lu - size %lu", video_fb.width, video_fb.height, video_fb.bpp, video_fb.pitch, video_fb.size);
    log("Kernel start.");
    log("Kernel compiled on %s at %s.", __DATE__, __TIME__);

    x86_64_gdt_load();
    x86_64_idt_make();
    x86_64_idt_load();

    bool status = acpi_init();
    if (!status)
        panic("ACPI could not be initialized!");

    x86_64_pic_disable();
    x86_64_ioapic_init();
    x86_64_fpu_init();

    kernel_main();
}
