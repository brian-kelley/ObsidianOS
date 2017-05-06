bits 32

section .text

global _start
global _startDraw

extern kernel_main
extern vidtest

_start:
  ; set up all data segments to GDT descriptor 0x10 (flat data, full address space)
  ; stack will grow downward from start of start of bootloader
  ; can't write over any of bootloader because it contains permanent GDT
  mov ebp, 0x9DD00
  mov esp, ebp
  jmp vidtest
	;jmp _kernel_main
  ; kernel_main should never return, but in case it does...
  _startDraw:
  xor eax, eax
  .loop:
  mov dword [0xA0000 + eax], 0x000f000f
  add eax, 4
  cmp eax, 64000
  jne .loop

  .hang:
  hlt
  jmp .hang

