global loadIDT
; the actual input interrupt handlers (ending with iret)
global keyboardInterrupt
global mouseInterrupt
global rtcInterrupt
global pass
global ioWait

extern keyPressed
extern keyboardHandler
extern mouseHandler
extern rtcHandler
extern idtDesc
extern puts

section .text
loadIDT:
  cli
  lidt [idtDesc]
  sti
  ret

keyboardInterrupt:
  cli
  cld
  call keyboardHandler
  sti
  iret

mouseInterrupt:
  cli
  cld
  call mouseHandler
  sti
  iret

rtcInterrupt:
  cli
  call rtcHandler
  sti
  iret

ioWait:
  jmp .l1
  .l1:
  jmp .l2
  .l2:
  ret

pass:
  push .str
  call puts
  add esp, 4
  iret

.str: db 'pass', 0
