#!/bin/bash
set -e

LYKOS_ISO=".chariot-cache/recipes/custom/image/install/lykos.iso"
ARCH="${1:-x86_64}"      # Fallback to x86_64 if no arch is provided.
OVMF="qemu/ovmf-${ARCH}"

QEMU_FLAGS=(
    -s -S
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

# Run QEMU
if [ "$ARCH" = "x86_64" ]; then
    qemu-system-x86_64 -M q35 "${QEMU_FLAGS[@]}"
elif [ "$ARCH" = "aarch64" ]; then
    qemu-system-aarch64 -M virt -cpu cortex-a72 -device ramfb -device qemu-xhci -device usb-kbd -device usb-mouse "${QEMU_FLAGS[@]}"
fi
