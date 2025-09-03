#!/usr/bin/env bash

ADDRESS=0x$1

addr2line -fai -e .chariot-cache/recipes/custom/kernel/install/kernel.elf $ADDRESS
