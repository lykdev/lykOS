#!/bin/bash
set -e  # Exit on error

clang -target x86_64-elf -ffreestanding -fno-pic -m64 -nostdlib -Wall -Wextra -mcmodel=large -Imodules -Ikernel/src -c modules/ps2kb/main.c -o modules/ps2kb/main.elf

echo "Build complete: main.elf"

