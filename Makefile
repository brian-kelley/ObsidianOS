CC=i386-elf-gcc
CFLAGS=-c -Wall -Wextra -ffreestanding -std=gnu99 -O0
OSNAME=os

CSOURCES=$(wildcard *.c)
ASOURCES=$(wildcard *.s)

COBJECTS=$(CSOURCES:.c=.o) 
AOBJECTS=$(ASOURCES:.s=.o)

EXECUTABLE=$(OSNAME).bin

all: $(COBJECTS) $(AOBJECTS) $(EXECUTABLE)
	qemu-system-i386 -cdrom $(OSNAME).iso

clean:
	rm *.o
	rm $(OSNAME).bin
	rm $(OSNAME).iso

$(EXECUTABLE): $(AOBJECTS) $(COBJECTS)
	$(CC) -T linker.ld -o $(OSNAME).bin -ffreestanding -std=gnu99 -nostdlib $(AOBJECTS) $(COBJECTS) -lgcc
	cp $(OSNAME).bin isodir/boot/$(OSNAME).bin
	grub-mkrescue -o $(OSNAME).iso isodir	

.c.o:
	$(CC) $(CFLAGS) $< -o $@
.s.o:
	i386-elf-as $< -o $@
