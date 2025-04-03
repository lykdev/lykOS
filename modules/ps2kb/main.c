#include <lib/def.h>
#include <module.h>

#define PS2_DATA_PORT  0x60
#define PS2_STATUS_REG 0x64
#define PS2_CMD_REG    0x64

#define IRQ 1

extern void abc();

void ps2_init()
{
    abc();
    asm volatile("int $0x3");
}

MODULE_INSTALL(ps2_init);