#!/bin/bash
set -e

LYKOS_ISO=".chariot-cache/recipes/custom/image/install/lykos.iso"
ARCH="${1:-x86_64}"      # Fallback to x86_64 if no arch is provided.
OVMF="qemu/ovmf-${ARCH}"
DISK="qemu/disk.img"

QEMU_FLAGS=(
    -m 2G
    -smp 1
    -no-shutdown
    -no-reboot
    -cdrom "$LYKOS_ISO"
    -boot d
    -d int
    -D qemu/log.txt
    -serial file:/dev/stdout
    -monitor stdio
    -bios "$OVMF"
)

# Fetch OVMF if missing
if [ ! -f "$OVMF" ]; then
    mkdir -p "$(dirname "$OVMF")"
    if [ "$ARCH" = "x86_64" ]; then
        curl -Lo "$OVMF" https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd
    elif [ "$ARCH" = "aarch64" ]; then
        curl -Lo "$OVMF" https://retrage.github.io/edk2-nightly/bin/RELEASEAARCH64_QEMU_EFI.fd
    else
        echo "Unsupported ARCH: $ARCH"
        exit 1
    fi
fi

if [ ! -f "$DISK" ]; then
    mkdir -p "$(dirname "$DISK")"
    qemu-img create -f raw ${DISK} 128M
fi

# Run QEMU
if [ "$ARCH" = "x86_64" ]; then
    qemu-system-x86_64 -M q35 "${QEMU_FLAGS[@]}" -drive id=disk,file=${DISK},if=none,format=raw -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0
elif [ "$ARCH" = "aarch64" ]; then
    qemu-system-aarch64 -M virt -cpu cortex-a72 -device ramfb -device qemu-xhci -device usb-kbd -device usb-mouse "${QEMU_FLAGS[@]}"
fi
