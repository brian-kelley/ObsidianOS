#!/usr/bin/python

import glob
import string
import os
import sys
import subprocess

# Set up constants

cflags = "-std=c99 -Os -m32 -ffreestanding -nostdlibinc -Istdlib/include -c"
# format for assembling machine code (TODO: match output of C compiler on mac and linux)
objformat = "macho32"
# TODO: set these based on platform (currently just for mac)
asm = "nasm"
cc = "gcc"
ld = "ld"
nm = "gnm"
objcopy = "gobjcopy"
objdump = "gobjdump"
emulator = "qemu-system-i386"

# place object files and temporary files in /build
build = "temp/"

objsModified = False

# get most recent modify time in stdlib/include
includeModifyTime = 0
for f in glob.iglob("stdlib/include/*.h"):
    iterTime = os.path.getmtime(f)
    if iterTime > includeModifyTime:
        includeModifyTime = iterTime

def run(cmd):
    rv = subprocess.call(cmd, shell=True)
    if rv != 0:
        print("!!!! Error executing command: \"" + cmd + "\"")
        sys.exit(1)

# Compile C or assembly source file (only if source modified more recently than output, like make)
def compile(f):
    isAsm = True
    if f[-2:] == ".c":
        # C file
        isAsm = False
    elif f[-4:] != ".asm":
        print("Unrecognized source file format: \"" + f + "\"")
        sys.exit(1)
    output = build + os.path.splitext(os.path.basename(f))[0] + ".o"
    # get f's modification time
    sourceTime = os.path.getmtime(f)
    # get output's modification time, if it exists
    if not os.path.isfile(output) or os.path.getmtime(output) < sourceTime or (not isAsm and includeModifyTime > os.path.getmtime(output)):
        # need to compile the file
        if isAsm:
            print("Assembling file: " + f)
            run(asm + " -f" + objformat + " " + f + " -o " + output)
        else:
            print("Compiling file: " + f)
            run(cc + " " + cflags + " " + f + " -o " + output)
        objsModified = True
    return output

# build libc objects

if not os.path.isdir(build):
    os.mkdir(build)

objs = []

for f in glob.glob("stdlib/src/*.c") + glob.glob("stdlib/src/*.asm") + glob.glob("kernel/src/*.c") + glob.glob("kernel/src/*.asm"):
    objs.append(compile(f))

print("Linking all object files...")
ldCommand = ld + " -r "
for o in objs:
    ldCommand += o + " "
ldCommand += " -o " + build + "system.o"
run(ldCommand)

# get locations of symbols in the full system blob

print("Getting symbol list...")
run(nm + " --demangle -v " + build + "system.o &> " + build + "symbols.txt")

# get flat binary version of system.o

print("Getting system binary...")
run(objcopy + " --set-start 0x500 -O binary " + build + "system.o " + build + "system.bin")

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
run(objdump + " -h " + build + "system.o > " + build + "sections.txt")
# imageSize = largest LMA + size for any section

imageSize = 0
for line in open(build + "sections.txt").readlines():
    words = string.split(line)
    if len(words) > 1 and words[0].isdigit():
        sectionEnd = int(words[2], 16) + int(words[4], 16)
        if sectionEnd > imageSize:
            imageSize = sectionEnd

imageFileSize = os.stat(build + "system.bin").st_size
imageSectors = (imageFileSize + 511) / 512

print("On-disk system image is " + hex(imageFileSize) + " bytes or " + str(imageSectors) + " sectors.")
print("In-memory system image is " + hex(imageSize) + " bytes (free memory starts at " + hex(imageSize + 0x500) + ")")

# Write into temporary bootloader assembly file with those values
print("Creating bootloader...")
bootAsm = open(build + "boot.asm", 'w')
bootAsm.write("org 0x3E\n");
bootAsm.write("mov ax, " + str(imageSectors) + '\n')
# FAT16 volume layout always the same, data area starts at sector 32 (see utils/ImageBuilder.c)
bootAsm.write("mov bx, 32\n")
bootAsm.write("mov cx, " + hex(kernelEntry >> 16) + '\n')
bootAsm.write("mov dx, " + hex(kernelEntry & 0xFFFF) + '\n')
# then, include all the original lines of boot/boot.asm
for line in open("boot/boot.asm").readlines():
    if line == "JUMP_TO_KERNEL\n":
        bootAsm.write("jmp 0x08:" + hex(kernelEntry))
    else:
        bootAsm.write(line)
bootAsm.close()

print("Assembling bootloader...")
run(asm + " " + build + "boot.asm -fbin -o " + build + "boot.bin");
print("Building system image...")
os.chdir("utils")
run(cc + " ImageBuilder.c -o ImageBuilder.exe")
run("./ImageBuilder.exe --boot ../" + build + "boot.bin --kernel ../" + build + "system.bin")
run("mv obsidian.img ..")
os.chdir("..")
print("Done... starting emulator.")
run(emulator + " -no-reboot -drive format=raw,media=disk,file=obsidian.img")

