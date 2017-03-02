bits 32

fibb:
push ebp
mov ebp, esp
mov eax, 1
mov ebx, 1
mov ecx, 5
.l1:
mov edx, ebx
add eax, ebx
mov ebx, edx
loop .l1
pop ebp
ret

main:
push ebp
mov ebp, esp
call fibb
pop ebp
xor eax, eax
ret

