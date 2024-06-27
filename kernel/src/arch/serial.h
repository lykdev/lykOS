#pragma once

/**
 * @note The `x86_64` implementation for this interface is only meant to work with the following emulators: QEMU & Bochs.
 */

void serial_send_char(char c);

void serial_send_str(const char *str);
