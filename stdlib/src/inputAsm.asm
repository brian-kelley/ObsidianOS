global loadIDT
; the actual input interrupt handlers (ending with iret)
global keyboardISR
global mouseISR
global rtcISR
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

keyboardISR:
  cld
  call keyboardHandler
  sti
  iret

mouseISR:
  call mouseHandler 
  sti
  iret

rtcISR:
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
