#!/bin/bash

ISO_NAME=lykos.iso

ARCH=$1

QEMU_FLAGS_X86_64="-M q35 -m 256M -smp 4 \
           -d int -M smm=off -D ./log.txt \
           -no-shutdown -no-reboot -monitor stdio" 

get_ovmf() {
    if [ $ARCH = x86_64 ]; then
        if [ ! -e "ovmf-x86_64" ]; then
            mkdir -p ovmf-x86_64
            curl -Lo ovmf-x86_64/OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd
        fi
    elif [ $ARCH = aarch64 ]; then
        if [ ! -e "ovmf-aarch64" ]; then
            mkdir -p ovmf-aarch64
            curl -Lo ovmf-aarch64/OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEAARCH64_QEMU_EFI.fd
        fi
    fi
    
}

start_qemu() {
    if [ $ARCH = x86_64 ]; then
        qemu-system-x86_64.exe -bios ovmf-x86_64/OVMF.fd -cdrom bin/$ISO_NAME -boot d $QEMU_FLAGS_X86_64
    elif [ $ARCH = aarch64 ]; then
        qemu-system-aarch64.exe -M virt -cpu cortex-a72 -device ramfb -device qemu-xhci -device usb-kbd -m 256M -bios ovmf-aarch64/OVMF.fd -cdrom bin/$ISO_NAME -boot d -monitor stdio
    fi
}

get_ovmf
start_qemu
