#!/usr/bin/env python3

import os
import sys
import subprocess

if len(sys.argv) < 4:
    print("Usage: python build.py <ARCH> <INCLUDE_DIR> <BUILD_DIR>")
    sys.exit(1)

if sys.argv[1] == "x86_64":
    TARGET = "x86_64-elf"
else:
    print(f"Unsupported arch: {sys.argv[1]}")
    sys.exit(1)

INCLUDE_DIR = sys.argv[2]
BUILD_DIR = sys.argv[3]

CFLAGS = [
    "-target", TARGET,
    "-ffreestanding",
    "-fno-pic",
    "-m64",
    "-std=gnu23",
    "-nostdlib",
    "-Wall",
    "-Wextra",
    "-mcmodel=large",
    "-mgeneral-regs-only",
    "-O2",
    f"-I{INCLUDE_DIR}"
]

for module in os.listdir(os.path.dirname(__file__)):
    module_path = os.path.join(os.path.dirname(__file__), module)
    if not os.path.isdir(module_path):
        continue

    sources = [
        os.path.join(module_path, f)
        for f in os.listdir(module_path)
        if f.endswith(".c")
    ]

    if not sources:
        continue

    object_files = []
    for src in sources:
        obj = os.path.join(BUILD_DIR, os.path.basename(src).replace(".c", ".o"))
        subprocess.run(["clang", *CFLAGS, "-c", src, "-o", obj], check=True)
        object_files.append(obj)

    output = os.path.join(BUILD_DIR, module + ".elf")
    subprocess.run(["ld.lld", "-r", "-o", output, *object_files], check=True)

    for obj in object_files:
        os.remove(obj)
