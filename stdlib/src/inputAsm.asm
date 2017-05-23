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
  pusha
  cld
  call keyboardHandler
  popa
  sti
  iret

mouseISR:
  pusha
  cld
  call mouseHandler 
  popa
  sti
  iret

rtcISR:
  pusha
  cld
  call rtcHandler
  popa
  sti
  iret

ioWait:
  jmp .l1
  .l1:
  jmp .l2
  .l2:
  ret

pass:
  ;push .str
  ;call puts
  ;add esp, 4
  pusha
  cld
  ; signal EOI on master PIC
  mov al, 0x20
  mov dx, 0x20
  out dx, al
  popa
  sti
  iret

.str: db 'pass', 0

