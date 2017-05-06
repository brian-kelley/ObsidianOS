bits 32

section .text

global _start
extern kernel_main

_start:
  ; set up all data segments to GDT descriptor 0x10 (flat data, full address space)
  ; start stack at highest possible address (immediately below video mem)
  mov ebp, 0xA0000
  mov esp, ebp
  sti
  xor ecx, ecx
  .drawLoop:
  mov dword [ebp + ecx], ecx
  add ecx, 4
  cmp ecx, 64000
  jne .drawLoop
  jmp .hang
	call kernel_main 
  ; kernel_main should never return, but in case it does...
  .hang:
  hlt
  jmp .hang

