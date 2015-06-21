CC=i386-elf-gcc
CFLAGS=-c -Wall -Wextra -ffreestanding -std=gnu99 -O2
OSNAME=goldos

#Kernel sources/objects
KCS=$(wildcard kernel/*.c)
KAS=$(wildcard kernel/*.s)
KCO=kernel/build/$(KCS:.c=.o)
KAO=kernel/build/$(KAS:.asm=.o)

SAS=$(wildcard stdlib/src/*.s)
SCS=$(wildcard stdlib/src/*.c)
SAO=$(SAS:.asm=.o)
SCO=$(SCS:.c=.o)

all: kernel
	qemu-system-i386 -cdrom $(OSNAME).iso

kernel: libc
	$(CC) -T linker.ld -o $(OSNAME).bin -ffreestanding -std=gnu99 -nostdlib $(KAO) $(KCO) -lc -lgcc
	mv $(OSNAME).bin isodir/boot
	grub-mkrescue -o $(OSNAME) isodir
	
libc: $(SCS) $(SAS) $(SCO) $(SAO)
	ar rcs libc.a $(SAO) $(SCO)

clean:
	rm $(OSNAME).iso

.c.o:
	$(CC) $(CFLAGS) -Istdlib/include $< -o $@ 
.s.o:
	nasm -f elf32 $< -o $@
