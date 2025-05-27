#include "ps2kb.h"

#include <arch/int.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/devices/ioapic.h>
#include <common/log.h>
#include <fs/vfs.h>

#define PS2_DATA_PORT 0x60
#define KB_IRQ 1

static void ps2kb_irq_handler()
{
    u8 scancode = x86_64_io_inb(PS2_DATA_PORT);

    log("Scancode: 0x%02X\n", scancode);
}

void ps2kb_setup()
{
    // Wait until input buffer is clear
    while (x86_64_io_inb(0x64) & 0x02)
        ;

    // Send command to read command byte
    x86_64_io_outb(0x64, 0x20);
    while (!(x86_64_io_inb(0x64) & 0x01))
        ;
    u8 cmd = x86_64_io_inb(0x60);

    // Modify: enable IRQ1 (bit 0), enable keyboard clock (bit 1)
    cmd |= 0x01; // IRQ1 enable
    cmd |= 0x10; // clock enable

    // Write modified command byte
    while (x86_64_io_inb(0x64) & 0x02)
        ;
    x86_64_io_outb(0x64, 0x60);
    while (x86_64_io_inb(0x64) & 0x02)
        ;
    x86_64_io_outb(0x60, cmd);

    // Enable first PS/2 port
    while (x86_64_io_inb(0x64) & 0x02)
        ;
    x86_64_io_outb(0x64, 0xAE);

    x86_64_ioapic_map_legacy_irq(KB_IRQ, 0, false, false, 0x30);

    vfs_node_t *dev_dir = vfs_lookup("/dev");
    vfs_node_t *char_dev = dev_dir->dir_ops->create(dev_dir, VFS_NODE_CHAR, "kb");
}
