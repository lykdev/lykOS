#!/usr/bin/env bash

objdump initrd/main.elf \
    --disassemble=$1 -wrC \
    --visualize-jumps=color \
    --disassembler-color=on