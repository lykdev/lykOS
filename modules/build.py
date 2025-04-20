import os
import sys
import subprocess

if len(sys.argv) < 3:
    print("Usage: python build.py <ARCH> <KERNEL_SRC_DIR>")
    sys.exit(1)

if sys.argv[1] == "x86_64":
    TARGET = "x86_64-elf"
else:
    print(f"Unsupported arch: {sys.argv[1]}")
    sys.exit(1)

KERNEL_SRC_DIR = sys.argv[2]
CURRENT_DIR = os.path.dirname(__file__)
SRC_DIR = os.path.join(CURRENT_DIR, "src")
BIN_DIR = os.path.join(CURRENT_DIR, "bin")
os.makedirs(BIN_DIR, exist_ok=True)
INCLUDE_DIR = os.path.join(CURRENT_DIR, "../kernel/src")

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
    "-O2",
    f"-I{KERNEL_SRC_DIR}"
]

for module in os.listdir(CURRENT_DIR):
    module_path = os.path.join(CURRENT_DIR, module)
    if not os.path.isdir(module_path):
        continue

    sources = [
        os.path.join(module_path, f)
        for f in os.listdir(module_path)
        if f.endswith(".c")
    ]

    if not sources:
        continue

    output = os.path.join(BIN_DIR, module + ".elf")
    subprocess.run(["clang", *CFLAGS, "-c", *sources, "-o", output], check=True)
