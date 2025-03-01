#pragma once

/**
 * @note The `x86_64` implementation for this interface is only meant to work
 * with the following emulators: QEMU & Bochs.
 */

void arch_serial_send_char(char c);

void arch_serial_send_str(const char *str);
