CC=i386-elf-gcc
CFLAGS=-c -Wall -Wextra -ffreestanding -std=gnu99 -O2
OSNAME=os

CSOURCES=$(wildcard *.c)
ASOURCES=$(wildcard *.s)

COBJECTS=$(CSOURCES:.c=.o) 
AOBJECTS=$(ASOURCES:.s=.o)

STDCSRC=$(wildcard stdlib/*.c)
STDASRC=$(wildcard stdlib/*.s)
STDCOBJ=$(STDCSRC:.c=.o)
STDAOBJ=$(STDASRC:.s=.o)

EXECUTABLE=$(OSNAME).bin

all: $(EXECUTABLE)
	qemu-system-i386 -cdrom $(OSNAME).iso

libc: $(STDCSRC) $(STDASRC) $(STDCOBJ) $(STDAOBJ)
	ar rcs libc.a port.o $(STDCOBJ) $(STDAOBJ)

clean:
	rm *.o
	rm $(OSNAME).bin
	rm $(OSNAME).iso

$(EXECUTABLE): $(CSOURCES) $(ASOURCES) $(AOBJECTS) $(COBJECTS)
	$(CC) -T linker.ld -o $(OSNAME).bin -ffreestanding -std=gnu99 -nostdlib $(AOBJECTS) $(COBJECTS) -lgcc
	cp $(OSNAME).bin isodir/boot/$(OSNAME).bin
	grub-mkrescue -o $(OSNAME).iso isodir	

.c.o:
	$(CC) $(CFLAGS) $< -o $@
.s.o:
	i386-elf-as $< -o $@
