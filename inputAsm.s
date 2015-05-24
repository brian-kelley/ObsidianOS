.global loadIDT
.global setupIDT
.global enableKeyboard
.global keyboardHandler
.extern keyPressed

.section .text
loadIDT:
	movl 4(%esp), %edx
	lidt (%edx)
	sti
	ret
	
setupIDT:
	mov $0x11, %al
	mov $0x20, %dx
	out %al, %dx
	mov $0xA0, %dx
	out %al, %dx
	
	mov $0x20, %al
	mov $0x21, %dx
	out %al, %dx
	mov $0x28, %al
	mov $0xA1, %dx
	out %al, %dx
	
	mov $0, %al
	mov $0x21, %dx
	out %al, %dx
	mov $0, %al
	mov $0xA1, %dx
	out %al, %dx
	
	mov $0x1, %al
	mov $0x21, %dx
	out %al, %dx
	mov $0x1, %al
	mov $0xA1, %dx
	out %al, %dx
	
	mov $0xFF, %al
	mov $0x21, %dx
	out %al, %dx
	mov $0xFF, %al
	mov $0xA1, %dx
	out %al, %dx
	ret
	
enableKeyboard:
	mov $0x21, %dx
	mov $0xFD, %al
	out %al, %dx
	ret

keyboardHandler:
	/* Let bl be status and bh be keycode */
	mov $0x20, %al
	mov $0x20, %dx
	outb %al, %dx
	mov $0x64, %dx
	in %dx, %al
	mov $1, %ah
	test %al, %ah
	je .end
	mov $0x60, %dx
	in %dx, %al
	call keyPressed
	.end:
	iret
