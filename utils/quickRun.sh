cd ../boot
nasm -fbin Stage1.asm -o Stage1.bin
cd ../utils
./ImageBuilder --boot ../boot/Stage1.bin
qemu-system-i386 obsidian.img

