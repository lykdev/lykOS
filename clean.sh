#!/bin/bash

clean_all() {
    rm -rf limine/ ovmf-aarch64/ ovmf-x86_64/
}

rm -rf log.txt
rm -rf iso_root bin
make -C kernel clean

if [ "$1" = "all" ]; then
    clean_all
fi

clear