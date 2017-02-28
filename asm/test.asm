bits 32
mov eax, [myvar1]
mov edx, [myvar2]
; eax *= edx (signed)
imul edx
;Make some variables
myvar1: dd 5
myvar2: dd -2

