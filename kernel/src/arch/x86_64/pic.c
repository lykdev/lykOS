#include "pic.h"

#include <arch/x86_64/io.h>

#define PIC_CMD_MASTER 0x20
#define PIC_DATA_MASTER 0x21
#define PIC_CMD_SLAVE 0xA0
#define PIC_DATA_SLAVE 0xA1

void pic_disable()
{
    // Mask all interrupts.
    x86_64_io_outb(PIC_DATA_MASTER, 0xFF);
    x86_64_io_outb(PIC_DATA_SLAVE, 0xFF);
}