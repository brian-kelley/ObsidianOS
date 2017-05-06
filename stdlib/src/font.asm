bits 32

global fontbin
global _vidtest

section .text
_vidtest:
mov esi, 0xA0000
xor ecx, ecx
.loop:
mov dword [esi + ecx], ecx
add ecx, 4
cmp ecx, 64000
jl .loop
.halt:
hlt
jmp .halt

section .data

align 4

; Font bitmap credit: https://github.com/dhepper/font8x8
; To get glyph for character c, take 8 bytes starting at address:
; fontbin + (c - '!') << 3
fontbin: incbin "stdlib/src/font.bin"


