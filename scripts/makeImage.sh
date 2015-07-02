#!/bin/bash
#If freshDisk.img doesn't exist, create a fresh one with GRUB 2 and a FAT16 bootable primary partition set up.
#If disk.img doesn't exist, copy freshDisk.img to disk.img
#Otherwise don't modify disk.img
#create empty disk
sudo dd if=/dev/zero of=../disk.img count=32768 bs=512
./fdiskParams | sudo fdisk ../disk.img
sudo losetup /dev/loop0 ../disk.img
sudo losetup /dev/loop1 ../disk.img -o 1048576
sudo mkdosfs -F16 -f 2 /dev/loop1
sudo mount /dev/loop1 /mnt
sudo grub-install --root-directory=/mnt --no-floppy --modules="normal part_msdos fat multiboot" /dev/loop0
#copy grub os list and kernel to disk's boot directory
sudo cp isodir/boot/grub/grub.cfg /mnt/boot/grub
sudo cp isodir/boot/goldos.bin /mnt/boot
sudo umount /dev/loop1
sudo losetup -d /dev/loop1
sudo losetup -d /dev/loop0

