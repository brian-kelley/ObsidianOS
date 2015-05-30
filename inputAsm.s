.global loadIDT
.global keyboardInterrupt
.extern keyPressed
.extern drawNum
.extern keyboardHandler

.section .text
loadIDT:
	movl 4(%esp), %edx
	lidt (%edx)
	sti
	ret

keyboardInterrupt:
	cli
	pusha
	mov %esp, %ebp
	cld
	call keyboardHandler
	popa
	sti
	iret
