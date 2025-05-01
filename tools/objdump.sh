#!/usr/bin/env bash

ADDRESS=0x$1

objdump .chariot-cache/recipes/package/apps/install/usr/bin/test.elf \
    -d -wrC \
    --visualize-jumps=color \
    --disassembler-color=on \
    --start-address=$ADDRESS \
    --stop-address=0x$(printf '%x' $(($ADDRESS + 128)))
