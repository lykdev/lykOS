#include <arch/serial.h>

#include <mm/kmem.h>
#include <sys/devices/dev.h>
#include <lib/string.h>

// #define PS2_DATA_PORT  0x60
// #define PS2_STATUS_REG 0x64
// #define PS2_CMD_REG    0x64

// #define IRQ 1

void __module_install()
{
    log("Hello world from inside module!");
}

void __module_destroy()
{
    
}

void __module__probe()
{
    
}