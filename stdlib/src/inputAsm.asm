global loadIDT
; the actual input interrupt handlers (ending with iret)
global keyboardISR
global mouseISR
global rtcISR
global pass
global generalISR
global ioWait
global inttest

extern keyPressed
extern keyboardHandler
extern mouseHandler
extern rtcHandler
extern idtDesc
extern puts
extern printf

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

generalISR:
  push generalHandlerMsg
  call printf
  .hang:
  hlt
  jmp .hang

ioWait:
  jmp .l1
  .l1:
  jmp .l2
  .l2:
  ret

pass:
  pusha
  cld
  ; signal EOI on master PIC
  mov al, 0x20
  mov dx, 0x20
  out dx, al
  popa
  sti
  iret

inttest:
  push dword 0xCAFEBABE
  push dword 0xDEADBEEF
  int 17 

generalHandlerMsg: db 'Exception, 16 bytes at esp:', 0x0A, '%#08x', 0x0A, '%#08x', 0x0A, '%#08x', 0x0A, '%#08x', 0

