; ObsidianOS bootloader
; First stage must fit in 446 bytes (image builder will check)
; First stage's main job is to load 2nd stage loaded at 0x9C000 (624 K)
; Maybe also show a message on screen.

; Jobs for 1st stage loader:
; -Set up stack, segment regs
; -Get drive geometry (dl has disk # on boot but is always 0x80)
; -Load 2nd stage bootloader at 0x9C000

bits 16

; Can't rely on either starting cs:ip being 0x7C0:0 or 0:0x7C00

; set up stack and data right above the 1st stage position
; stack will grow towards the data but will never reach it
mov ax, 0x7E0
mov ds, ax
mov ss, ax
mov bp, 0x1000
mov sp, bp
; get disk geometry
mov ah, 0x8
xor bx, bx
mov es, bx
mov di, bx
int 0x13
; save cylinders to 0x7E00, heads to 0x7E02, sectors/track to 0x7E04
; sectors/track in cl 5:0
xor ax, ax
mov al, cl
and al, 0x3F
mov [4], ax
; heads in dh
mov al, dh
mov [2], ax
; cylinders: in cl 7:6 (high), and ch 7:0 (low)
mov al, ch
shr cl, 6
mov ah, cl
mov [0], ax

; set video mode to 13h
mov ah, 0
mov al, 0x13
int 0x10

; show some pixels
mov ax, 0xA000
push ds
mov ds, ax

xor cx, cx
mov dx, 64000

drawLoop:
mov bx, cx
mov [bx], cl
inc cx
cmp cx, dx
jne drawLoop

pop ds

; read the 8k 2nd stage bootloader from FAT filesystem to 0x9C000
mov ax, 0x9C00
mov es, ax

; prepare to read 16 sectors starting at 32 to es:0
mov ax, 32
mov bx, 0
mov cx, 16

readLoop:
push ax
push cx
call readLBA
pop cx
pop ax
inc ax
add bx, 512
loopnz readLoop

hlt

; readLBA reads the sector ax (LBA) to es:bx
; handles the LBA -> CHS conversion
; preserves bx
; note: div z ==> ax /= z, dx = ax % z
readLBA:
  push bp
  mov bp, sp
  ; [bp - 2] = ax = LBA
  push ax
  ; ax now free for computations
  div word [4]
  ; ax = temp, dx = CHS sector - 1
  inc dx
  ; dx = CHS sector, save it
  push dx
  div word [2]
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
  pop dx
  pop ax
  pop bp
  ret

