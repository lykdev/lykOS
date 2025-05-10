#include <arch/int.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/devices/ioapic.h>
#include <common/log.h>
#include <lib/def.h>
#include <sys/smp.h>
#include <tasking/sched.h>

#define PORT_DATA 0x60
#define PORT_CTRL 0x64

static const char sc_ascii[] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', /* Backspace */
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',     /* Enter */
    0, /* Ctrl */ 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, /* LShift */ '\\','z','x','c','v','b','n','m',',','.','/',
    0, /* RShift */ '*', 0, ' ', 0, /* Alt */ 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void keyboard_irq_handler()
{
    log("HANDLER");
    u8 scancode = x86_64_io_inb(PORT_DATA);

    if (scancode & 0x80)
    {
        // key released, ignore for now
    }
    else
    {
        char c = sc_ascii[scancode];
        if (c)
            log("%c", c);
    }
}

void __module_install()
{
    x86_64_ioapic_map_legacy_irq(1, 0, 0, 0, 1);
    arch_int_irq_register_handler(1, keyboard_irq_handler);
}

void __module_destroy()
{

}

void __module_probe()
{

}
