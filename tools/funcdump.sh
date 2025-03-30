#!/usr/bin/env bash

objdump kernel/bin/kernel.elf \
    --disassemble=$1 -wrC \
    --visualize-jumps=color \
    --disassembler-color=on