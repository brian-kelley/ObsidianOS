section .text

global _start
extern kernel_main

_start:
  ; start stack at highest possible address (immediately below video mem)
  mov ebp, 0xA0000
  mov esp, ebp
	call kernel_main 
  .hang: jmp .hang

