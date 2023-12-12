#!/bin/bash

# Exit the script if any command returns a non-zero status.
set -e

KERNEL_FILE=kernel/bin/kernel.elf
ISO_NAME=lykos.iso

ARCH=$1

build_kernel() {
    make -C kernel ARCH=$ARCH
}

get_limine() {
	if [ ! -e "limine" ]; then
		git clone https://github.com/limine-bootloader/limine.git --branch=v5.x-branch-binary --depth=1
		make -C limine CC="$(HOST_CC)"
	fi
}

build_iso() {
    rm -rf iso_root
	mkdir -p iso_root
	mkdir -p iso_root/EFI/BOOT

	cp -v $KERNEL_FILE \
			limine.cfg limine/limine-uefi-cd.bin iso_root/

	if [ $ARCH = "x86_64" ]; then
		cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/

		xorriso -as mkisofs \
			-no-emul-boot -boot-load-size 4 -boot-info-table \
			--efi-boot limine-uefi-cd.bin \
			-efi-boot-part --efi-boot-image --protective-msdos-label \
			iso_root -o bin/$ISO_NAME
	elif [ $ARCH = "aarch64" ]; then
		cp -v limine/BOOTAA64.EFI iso_root/EFI/BOOT/

		# Raspberry Pi 4 UEFI Firmware
		wget https://github.com/pftf/RPi4/releases/download/v1.35/RPi4_UEFI_Firmware_v1.35.zip
		unzip -d iso_root RPi4_UEFI_Firmware_v1.35.zip
		rm -rf RPi4_UEFI_Firmware_v1.35.zip

		xorriso -as mkisofs \
			--efi-boot limine-uefi-cd.bin \
			-efi-boot-part --efi-boot-image --protective-msdos-label \
			iso_root -o bin/$ISO_NAME
	fi

	rm -rf iso_root
}

clean() {
	rm -rf log.txt iso_root *.iso

	make -C kernel clean
}

if [ "$#" -eq 0 ]; then
  echo "No architecture provided. Please pass a value when invoking build.sh, e.g., './build.sh x86_64'. Options are 'x86_64' and 'aarch64')"
  exit 1
fi
mkdir -p bin
build_kernel
get_limine
build_iso