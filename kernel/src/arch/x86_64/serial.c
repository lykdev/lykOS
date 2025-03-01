#include "arch/serial.h"

#include <arch/x86_64/io.h>

#define COM1 0x3F8

void arch_serial_send_char(char c) { x86_64_io_outb(COM1, c); }

void arch_serial_send_str(const char *str)
{
    while (*str != '\0')
        arch_serial_send_char(*(str++));
}
