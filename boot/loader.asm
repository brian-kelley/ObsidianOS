; ObsidianOS bootloader
; First stage must fit in 446 bytes (image builder will check)
; First stage's main job is to load 2nd stage loaded at 0x9C000 (624 K)
; Maybe also show a message on screen.

; Jobs for 1st stage loader:
; -Set up stack, segment regs
; -Get drive geometry (dl has disk # on boot)
; -Load 2nd stage bootloader at 0x9C000

bits 16

org 0


mov ax, 0x7C0
mov cs, ax
mov ds, ax
; set up stack in same segment
xor 

