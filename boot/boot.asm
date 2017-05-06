; ObsidianOS bootloader
;
; First stage must fit in 446 bytes (image builder will check)
; Jobs for 1st stage loader:
; -Copy self to end of low memory to make room for system image
; -Set up stack, segment regs
; -Get drive geometry
; -Load system image sectors at 0x500
; -Enter protected mode with trivial GDT
; -Jump to kernel entry point
;
; Can't rely on either starting cs:ip being 0x7C0:0 or 0:0x7C00,
; so bootloader is completely location independent
;
; set up stack and data right above the 1st stage position
; stack will grow towards this data but will never reach it
;
; Note: initial conditions at boot:
; org 0x3E: the offset of code within boot sector
; ax = # of kernel sectors
; bx = first kernel sector (aka start of FS data area)
; cx:dx = kernel entry point (absolute, 32-bit, flat memory address)
;
; first, copy whole bootloader from 0x7C00 to 0x8000, then far jump to it

; temporarily save cx in sp
mov sp, cx
mov si, 0x9DD0
mov es, si
mov cx, 256
; zero di
xor di, di
; zero ds
mov ds, di
mov si, 0x7C00
; copy boot sector from 0x0:0x7C00 (ds:si) to 0x9DD0:0 (es:di)
rep movsw
; far jump to resume label in new copy
; note: build.py puts an "org 0x3E" at the start of this file, so actual total address is 0x9FD00 + 0x3E + resume
jmp 0x9DD0:resume

resume:

; restore cx
mov cx, sp
; set up ds, ss (512 bytes above 0x9FD00)
; only provide 256 bytes of data/stack for bootloader
mov si, 0x9DF0
mov ds, si
mov ss, si
mov bp, 0x100
mov sp, bp

; save ax/bx/cx
push ax
push bx
push cx
push dx
push word 0x08

; from now on:
; [bp - 2] = # of kernel sectors to read
; [bp - 4] = first kernel sector
; [bp - 6] = high word of kernel entry point
; [bp - 8] = low word of kernel entry point
; now, 0x08 as a dword, [bp - 8] provides the full 32-bit address

; get hard disk geometry
mov ah, 0x8
xor bx, bx
mov es, bx
xor di, di
mov dl, 0x80
int 0x13
; save heads to ds:0, sectors per track to ds:2
; sectors/track in cl 5:0
xor ax, ax
mov al, cl
and al, 0x3F
mov [2], ax
; heads in dh
mov al, dh
mov [0], ax

; set video mode to 13h
mov ah, 0
mov al, 0x13
int 0x10

; prepare to read system.bin (this sets load address)
; is always 0x500 (currently)
mov ax, 0x50
mov es, ax

; read first [bp - 2] sectors, starting at [bp - 4], at 0x500
xor ax, ax
xor bx, bx
readLoop:
push ax
add ax, [bp - 4]
call readLBA
pop ax
; must set es for each sector, in case system.bin is bigger than one full segment
; just add 512 >> 4 = 32 to es each loop iteration
; then leave bx = 0
mov dx, es
add dx, 32
mov es, dx
inc ax
cmp ax, [bp - 2]
jne readLoop

mov ax, 0x9DD0
mov ds, ax

; do a 32-bit add of gdt pointer within bootloader segment to [gdtAddr]
add word [gdtAddr], gdt
adc word [gdtAddr], 0

cli
lgdt [ds:gdtPointer]


; enable protected mode
mov eax, cr0
or al, 1
mov cr0, eax
; set code selector (word) in front of the entry point at [bp - 8]
;mov word [bp - 10], 0x08
; jump to kernel entry point (start)
; that will set up stack, enable interrupts and run kernel main

;stopasdf:
;hlt
;jmp stopasdf 
mov ax, 0x10
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

jmp 0x08:0x7C9B
;jmp far [bp - 10]

; readLBA reads the sector ax (LBA) to es:bx
; handles the LBA -> CHS conversion
; preserves bx
; note: div z ==> ax /= z, dx = ax % z

bits 16
readLBA:
  ; ax now free for computations
  xor dx, dx
  div word [2]
  ; ax = total "heads", dx = CHS sector - 1
  inc dl
  ; dl = CHS sector
  ; note: value <= 63
  mov cl, dl
  xor dx, dx
  div word [0]
  ; ax = CHS cylinder
  ; dx = CHS head
  mov dh, dl
  mov ch, al
  shr ax, 8
  shl ax, 6
  or cl, al
  mov ah, 2
  mov al, 1
  mov dl, 0x80
  int 0x13
  ret

; GDT descriptor struct (call lgdt [gdtPointer])
gdtPointer:
; size, in bytes (minus 1)
gdtSize:
dw 23
; flat 32-bit ptr to gdt (need to add 0x3E + gdt to this)
gdtAddr:
dd 0x9DD00

; the permanent, in-memory copy of the GDT in the bootloader copy at 0x8000
; gdt entry structure (8 bytes):
;   segment limit 0:15 (2 bytes)
;   base addr 0:23 (3 bytes)
;   flags (1 byte)
;   more flags + limit 16:19 (1 byte)
;   base addr 24:32 (1 byte)
gdt:
; null descriptor
dq 0
; code descriptor (selector 08h)
dw 0xFFFF
dw 0
db 0
db 0b10011010 ; access byte
db 0b11001111 ; flags and limit 16:19
db 0
; data descriptor (selector 10h)
dw 0xFFFF
dw 0
db 0
db 0b10010010 ; access byte
db 0b11001111 ; flags and limit 16:19
db 0

