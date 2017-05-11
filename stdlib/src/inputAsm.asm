global loadIDT
global keyboardInterrupt
global pass
global ioWait

extern keyPressed
extern keyboardHandler
extern idtDesc
extern puts

section .text
loadIDT:
  cli
  lidt [idtDesc]
  sti
  ret

keyboardInterrupt:
  cld
  call keyboardHandler
  iret

ioWait:
  jmp .l1
  .l1:
  jmp .l2
  .l2:
  ret

pass:
  iret

