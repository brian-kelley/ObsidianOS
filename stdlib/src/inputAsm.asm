global loadIDT
global keyboardInterrupt

extern keyPressed
extern drawNum
extern keyboardHandler

section .text

loadIDT:
  mov edx, [esp + 4]
	lidt [edx]
	ret

keyboardInterrupt:
	call keyboardHandler
	iret

