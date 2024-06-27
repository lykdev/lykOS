#include <arch/x86_64/io.h>

#define COM1 0x3F8

// ARCH INTERFACE

#include "arch/serial.h"

void serial_send_char(char c)
{
    io_outb(COM1, c);
}

void serial_send_str(const char *str)
{
    while (*str != '\0')
        serial_send_char(*str++); 
}