#include <arch/serial.h>

// #define PS2_DATA_PORT  0x60
// #define PS2_STATUS_REG 0x64
// #define PS2_CMD_REG    0x64

// #define IRQ 1

void __module_install()
{
    arch_serial_send_str("Hello from inside a kernel module!");
}

void __module_destroy()
{
    asm volatile("int $0x5");
}