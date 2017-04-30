global fontbin

section .data

align 4

; Font bitmap credit: https://github.com/dhepper/font8x8
; To get glyph for character c, take 8 bytes starting at address:
; fontbin + (c - '!') << 3
fontbin: incbin "stdlib/src/font.bin"

