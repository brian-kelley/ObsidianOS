#!/usr/bin/python

import glob
import string
import os

# TODO: set these based on platform (currently for mac)
asm = "nasm"
cc = "gcc"
ld = "ld"
nm = "gnm"
objcopy = "gobjcopy"
objdump = "gobjdump"


# build libc objects

# place object files and temporary files in /build
build = "temp/"

if not os.path.isdir(build):
    os.mkdir(build)

cflags = "-std=c99 -Os -m32 -ffreestanding -nostdlibinc -Istdlib/include -c "

objs = []

for f in glob.iglob("stdlib/src/*.c"):
    base = os.path.basename(f)
    print("Compiling stdlib C file:        " + base)
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
    print("Compiling kernel C file:        " + base)
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

print("Linking all object files...");
ldCommand = ld + " -r ";
for o in objs:
    ldCommand += o + " "
ldCommand += " -o " + build + "system.o"
os.system(ldCommand)

# get locations of symbols in the full system blob

print("Getting symbol list...");
os.system(nm + " --demangle -v " + build + "system.o &> " + build + "symbols.txt")

# get flat binary version of system.o

print("Getting system binary...");
os.system(objcopy + " --set-start 0x500 -O binary " + build + "system.o " + build + "system.bin")

# get entry point as pointer (to add to bootloader)
# symbol is "start" (in kernel/src/start.asm)
kernelEntry = 0
for line in open(build + "symbols.txt").readlines():
    words = string.split(line)
    if len(words) < 3:
        continue
    if words[2] == "start":
        kernelEntry = 0x500 + int(words[0], 16)
        print("System entry point (absolute addr): " + hex(kernelEntry))
        break

# get symbol table entries to determine full in-memory size
os.system(objdump + " -h " + build + "system.o > " + build + "sections.txt")
# imageSize = largest LMA + size for any section
imageSize = 0;
for line in open(build + "sections.txt").readlines():
    words = string.split(line);
    if len(words) > 1 and words[0].isdigit():
        sectionEnd = int(words[2], 16) + int(words[4], 16)
        if sectionEnd > imageSize:
            imageSize = sectionEnd

imageFileSize = os.stat(build + "system.bin").st_size
imageSectors = (imageFileSize + 511) / 512

print("On-disk system image is " + hex(imageFileSize) + " bytes or " + str(imageSectors) + " sectors.")
print("In-memory system image is " + hex(imageSize) + " bytes (free memory starts at " + hex(imageSize + 0x500) + ")")

# Write into temporary bootloader assembly file with those values
print("Creating bootloader...");
bootAsm = open(build + "boot.asm", 'w');
bootAsm.write("mov ax, " + imageSectors + '\n')
# TODO: will the start of FS data area be dynamic? Hardcode this for now.
bootAsm.write("mov bx, 32\n");
bootAsm.write("mov cx, " + hex(kernelEntry) + '\n')
# then, include all the original lines of boot/boot.asm
for line in open("boot/boot.asm").readlines():
    bootAsm.write(line)
bootAsm.close()
print("Assembling bootloader...")
os.system(asm + " " + build + "boot.asm -fbin -o " + build + "boot.bin")
print("Building system image...")
os.chdir("utils")
os.system(cc + " ImageBuilder.c -o ImageBuilder.exe")
os.system("./ImageBuilder.exe --boot ../" + build + "boot.bin --kernel ../" + build + "system.bin")
os.system("mv obsidian.img ..")
os.chdir("..")
print("Done. Run qemu-system-i386 obsidian.img to boot.")
