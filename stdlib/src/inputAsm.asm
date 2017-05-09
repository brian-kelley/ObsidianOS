global loadIDT
global keyboardInterrupt
global pass

extern keyPressed
extern keyboardHandler
extern idt

section .text
loadIDT:
	lidt [idt]
  sti
	ret

keyboardInterrupt:
	call keyboardHandler
	iret

pass:
  iret

