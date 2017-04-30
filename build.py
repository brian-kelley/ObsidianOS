#!/usr/bin/python

import glob
import os

# TODO: set these based on platform (currently for mac)
asm = "nasm"
cc = "gcc"
ld = "ld"
nm = "gnm"
objcopy = "gobjcopy"


# build libc objects

# place object files and temporary files in /build
build = "build/"

if not os.path.isdir(build):
    os.mkdir(build)

cflags = "-Os -m32 -ffreestanding -nostdlibinc -Istdlib/include -c "

objs = []

for f in glob.iglob("stdlib/src/*.c"):
    base = os.path.basename(f)
    print("Compiling stdlib C file: " + base)
    objPath = build + os.path.splitext(base)[0] + ".o"
    os.system(cc + " " + cflags + f + " -o " + objPath)
    objs.append(objPath)

for f in glob.iglob("stdlib/src/*.asm"):
    base = os.path.basename(f)
    print("Compiling stdlib assembly file: " + base)
    objPath = build + os.path.splitext(base)[0] + ".o"
    os.system(asm + " " + "-fmacho32 " + f + " -o " + objPath)
    objs.append(objPath)

# compile kernel C/assembly files

for f in glob.iglob("kernel/src/*.c"):
    base = os.path.basename(f)
    print("Compiling kernel C file: " + base)
    objPath = build + os.path.splitext(base)[0] + ".o"
    os.system(cc + " " + cflags + f + " -o " + objPath)
    objs.append(objPath)

for f in glob.iglob("kernel/src/*.asm"):
    base = os.path.basename(f)
    print("Compiling kernel assembly file: " + base)
    objPath = build + os.path.splitext(base)[0] + ".o"
    os.system(asm + " -fmacho32 " + f + " -o " + objPath)
    objs.append(objPath)

# link together all object files into a single large one

ldCommand = ld + " -r ";
for o in objs:
    ldCommand += o + " "
ldCommand += " -o " + build + "system.o"
os.system(ldCommand)

# get locations of symbols in the full system blob

os.system(nm + " --demangle -v build/system.o &> build/symbols.txt")

# get flat binary version of system.o

os.system(objcopy + " -O binary build/system.o build/system.bin")

