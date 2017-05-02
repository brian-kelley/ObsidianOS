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
; ax = # of kernel sectors
; bx = first kernel sector (aka start of FS data area)
; cx = kernel entry point (absolute memory address)
; dl = disk # (possibly used for getting drive geometry)
;
; first, copy whole bootloader from 0x7C00 to 0x9C000
; then jump to it

; temporarily save cx in sp
mov sp, cx
mov si, 0x7C0
mov ds, si
mov si, 0x9C00
mov es, si
; copy 446 bytes from ds:0 to es:0
mov cx, 446
xor si, si
mov di, si
rep movs
; far jump to resume label in new copy
jmp 0x9C00:resume

resume:

; restore cx
mov cx, sp
mov si, 0x7E0
mov ds, si
mov ss, si
mov bp, 0x1000
mov sp, bp

; save ax/bx/cx
push cx
push bx
push ax

mov bp, sp
; from now on:
; [bp + 0] = # of kernel sectors to read
; [bp + 2] = first kernel sector
; [bp + 4] = kernel entry point

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

mov ax, 0xB800
mov gs, ax

mov dx, 0x0700

xor bx, bx

clearScreen:
mov [gs:bx], dx
add bx, 2
cmp bx, 2000
jne clearScreen

; print cylinders, heads, sectors/track
mov al, [0]
xor bx, bx
call printNum
inc bx
mov al, [2]
call printNum
inc bx
mov al, [4]
call printNum

hlt

; set video mode to 13h
mov ah, 0
mov al, 0x13
int 0x10

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
;call readLBA
pop cx
pop ax
inc ax
add bx, 512
loopnz readLoop

;xor bx, bx
;mov word [es:bx], 0x000F
;mov word [es:bx + 2], 0x000F
;mov word [es:bx + 4], 0x000F

; show some pixels
mov ax, 0xA000
push ds
mov ds, ax

;xor bx, bx
;mov dx, 8192

drawLoop:
mov al, [es:bx]
mov [bx], bh
inc bx
cmp bx, dx
jne drawLoop

pop ds

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

; print number as ASCII char 33 + al
; print at column bx in 1st row
; this can effectively print numbers from 0 to 93
; preserves all regs
printNum:
  push bx
  add bx, bx
  add ax, 33
  mov [gs:bx], al
  sub ax, 33
  pop bx
  ret

