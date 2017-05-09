bits 32

section .text

global _start

extern kernel_main
extern loadEmptyIDT

_start:
  ; set up all data segments to GDT descriptor 0x10 (flat data, full address space)
  ; stack will grow downward from start of start of bootloader
  ; can't write over any of bootloader because it contains permanent GDT
  mov ebp, 0x9DD00
  mov esp, ebp
  ;call loadEmptyIDT
  ;sti
  jmp kernel_main
  ; kernel_main should never return, but in case it does...

  .hang:
  hlt
  jmp .hang

