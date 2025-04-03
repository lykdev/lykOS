#include <lib/def.h>
#include <module.h>

#define PS2_DATA_PORT  0x60
#define PS2_STATUS_REG 0x64
#define PS2_CMD_REG    0x64

#define IRQ 1

void ps2_init()
{
    asm volatile("int $0x3");
    // register_interrupt_handler(IRQ1, kb_irq_handler);

    // outb(PS2_CMD_REG, 0xAE); // Enable keyboard port
    // outb(PS2_DATA_PORT, 0xF4); // Enable scanning
}

MODULE_INSTALL(ps2_init);