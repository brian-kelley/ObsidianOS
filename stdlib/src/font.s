.global getFontPtr
.section .data
.type fontbin, @object
.align 4
fontbin:
.incbin "stdlib/src/font.bin"

.section .text
getFontPtr:
	mov $fontbin, %eax
	add $0x210, %eax
	ret
