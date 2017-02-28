.global loadIDT
.global keyboardInterrupt
.extern keyPressed
.extern drawNum
.extern keyboardHandler

.section .text
loadIDT:
	movl 4(%esp), %edx
	lidt (%edx)
	ret

keyboardInterrupt:
	call keyboardHandler
	iret

