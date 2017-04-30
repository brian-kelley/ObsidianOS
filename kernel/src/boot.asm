section .text

global _start
extern kernel_main

_start:
  ; start stack at highest possible address (immediately below video mem)
  mov esp, 0xA0000
	call kernel_main 
  .hang: jmp .hang

