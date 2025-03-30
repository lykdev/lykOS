#!/bin/bash
set -e  # Exit on error

clang -target x86_64-elf -ffreestanding -fno-pic -m64 -nostdlib -Wall -Wextra -c modules/main.c -o modules/main.elf

echo "Build complete: main.elf"

