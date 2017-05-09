global loadIDT
global keyboardInterrupt

extern keyPressed
extern keyboardHandler

section .text
loadIDT:
  mov edx, [esp + 4]
	lidt [edx]
  sti
	ret

keyboardInterrupt:
	call keyboardHandler
	iret

