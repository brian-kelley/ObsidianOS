CC=i386-elf-gcc
CFLAGS=-c -ffreestanding -std=gnu99 -Os
WARNINGS=-Wall -Wextra -Wno-strict-aliasing -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter
OSNAME=goldos
DESTIMAGE=../disk.img

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
KAS=$(wildcard kernel/src/*.asm)
KGS=$(wildcard kernel/src/*.s)
KCO=$(KCS:.c=.o)
KAO=$(KAS:.asm=.o)
KGO=$(KGS:.s=.o)

SAS=$(wildcard stdlib/src/*.asm)
SCS=$(wildcard stdlib/src/*.c)
SGS=$(wildcard stdlib/src/*.s)
SAO=$(SAS:.asm=.o)
SCO=$(SCS:.c=.o)
SGO=$(SGS:.s=.o)

all: kernel
	sudo scripts/makeImage.sh
	sudo qemu-system-i386 $(DESTIMAGE)

kernel: libc $(KCS) $(KCH) $(KAS) $(KGS) $(KCO) $(KAO) $(KGO)
	$(CC) -T linker.ld -o $(OSNAME).bin -static -ffreestanding -std=gnu99 -nostdlib -Os $(KAO) $(KCO) $(KGO) -L. -lc -lgcc
	mv $(OSNAME).bin isodir/boot
	grub-mkrescue -o $(OSNAME).iso isodir
	
libc: $(SCS) $(SAS) $(SGS) $(SCO) $(SAO) $(SGO)
	ar rcs libc.a $(SAO) $(SCO) $(SGO)

clean:
	rm -f stdlib/src/*.o
	rm -f kernel/src/*.o
	rm -f $(OSNAME).iso

checkDiskUtils:
	scripts/checkPrograms.sh

.c.o:
	$(CC) $(CFLAGS) $(WARNINGS) -Istdlib/include $< -o $@ 
.asm.o:
	nasm -f elf32 $< -o $@
.s.o:
	i386-elf-as $< -o $@
