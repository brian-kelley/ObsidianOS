global loadIDT
; the actual input interrupt handlers (ending with iret)
global keyboardInterrupt
global mouseInterrupt
global pass
global ioWait

extern keyPressed
extern keyboardHandler
extern mouseHandler
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

mouseInterrupt:
  cld
  call mouseHandler
  iret

ioWait:
  jmp .l1
  .l1:
  jmp .l2
  .l2:
  ret

pass:
  iret

