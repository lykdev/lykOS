#!/usr/bin/env python3

import os
import sys
import subprocess
import shlex

if len(sys.argv) < 3:
    print("Usage: python build.py <CC> <BUILD_DIR>")
    sys.exit(1)

CC = shlex.split(sys.argv[1])
BUILD_DIR = sys.argv[2]

CFLAGS = [
    "-static",
    "-std=gnu23",
    "-O2",
    "-g",
    "-v"
]

for app in os.listdir(os.path.dirname(__file__)):
    app_path = os.path.join(os.path.dirname(__file__), app)
    if not os.path.isdir(app_path):
        continue

    sources = [
        os.path.join(app_path, f)
        for f in os.listdir(app_path)
        if f.endswith(".c")
    ]

    if not sources:
        continue

    output = os.path.join(BUILD_DIR, app + ".elf")
    cmd = CC + CFLAGS + sources + ["-o", output]
    print(cmd)
    subprocess.run(cmd, check=True, stderr=subprocess.STDOUT)
