bits 32
and eax, eax ;this is an and instruction
and ebx, ebx ;here is another
label:
add eax, esp
add eax, [ebp + 8]

