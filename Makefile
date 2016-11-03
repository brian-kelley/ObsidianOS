CC=i386-elf-gcc
CFLAGS=-c -ffreestanding -std=gnu99 -Os
WARNINGS=-Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-strict-aliasing
OSNAME=goldos
DESTIMAGE=../disk.img

INCLUDE=-Istdlib/include -IDataStruct/include

#K = kernel
#S = stdlib
#C = c language
#A = assembly (NASM, .asm extension)
#G = GNU assembly (GAS, .s extension)
#S = source
#O = object file
#H = header

KCS=$(wildcard kernel/src/*.c)
KCH=$(wildcard kernel/src/*.h)
KAS=$(wildcard kernel/src/*.a)
KGS=$(wildcard kernel/src/*.s)
KCO=$(KCS:.c=.o)
KAO=$(KAS:.a=.o)
KGO=$(KGS:.s=.o)

SAS=$(wildcard stdlib/src/*.a)
SCS=$(wildcard stdlib/src/*.c)
SGS=$(wildcard stdlib/src/*.s)
SAO=$(SAS:.a=.o)
SCO=$(SCS:.c=.o)
SGO=$(SGS:.s=.o)

DataStructSources=$(wildcard DataStruct/src/*.c)
DataStructObjects=$(DataStructSources:.c=.o)

all: kernel
	sudo scripts/makeImage.sh
	sudo qemu-system-i386 $(DESTIMAGE)

kernel: libc libstruct $(KCS) $(KCH) $(KAS) $(KGS) $(KCO) $(KAO) $(KGO)
	$(CC) -T linker.ld -o $(OSNAME).bin -static -ffreestanding -std=gnu99 -nostdlib -Os $(KAO) $(KCO) $(KGO) -L. -lc -lstruct -lgcc
	mv $(OSNAME).bin isodir/boot
	grub-mkrescue -o $(OSNAME).iso isodir
	
libc: $(SCS) $(SAS) $(SGS) $(SCO) $(SAO) $(SGO) $(SINC)
	ar rcs libc.a $(SAO) $(SCO) $(SGO)

libstruct: $(DataStructObjects)
	ar rcs libstruct.a $(DataStructObjects)

clean:
	rm -f stdlib/src/*.o
	rm -f kernel/src/*.o
	rm -f $(OSNAME).iso

checkDiskUtils:
	scripts/checkPrograms.sh

.c.o:
	$(CC) $(CFLAGS) $(WARNINGS) $(INCLUDE) $< -o $@ 
.a.o:
	nasm -felf32 $< -o $@
.s.o:
	i386-elf-as $< -o $@
