; x87 FPU math.h functions

extern puts

global initFPU		; void initFPU
global sin
global sinf
global sinl
global cos
global cosf
global cosl
global tan
global tanf
global tanl
global sqrt
global sqrtf
global sqrtl
global atan
global atanf
global atanl
global atan2
global atan2f
global atan2l
global exp
global expf
global expl
global pow
global powf
global powl
global log
global logf
global logl
global log10
global log10f
global log10l
global frexp
global frexpf
global frexpl
global ldexp
global ldexpf
global ldexpl
global fabs
global fabsf
global fabsl
global modf
global modff
global modfl
global fmod
global fmodf
global fmodl
global ceil
global ceilf
global ceill
global floor
global floorf
global floorl
global fmod

section .text

initFPU:
    fninit
    ret

sin:
    ; x is the 8 bytes at first arg (esp + 4)
    fld qword [esp + 4]
    fsin
    ret

sinf:
    fld dword [esp + 4]
    fsin
    ret

sinl:
    ; x is a 10-byte float within 12 bytes
    fld tword [esp + 4]
    fsin
    ret

cos:
    fld qword [esp + 4]
    fcos
    ret

cosf:
    fld dword [esp + 4]
    fcos
    ret

cosl:
    fld tword [esp + 4]
    fcos
    ret

tan:
    fld qword [esp + 4]
    fptan
    fstp st0
    ret

tanf:
    fld dword [esp + 4]
    fptan
    fstp st0
    ret

tanl:
    fld tword [esp + 4]
    fptan
    fstp st0
    ret

sqrt:
    fld qword [esp + 4]
    fsqrt
    ret

sqrtf:
    fld dword [esp + 4]
    fsqrt
    ret

sqrtl:
    fld tword [esp + 4]
    fsqrt
    ret

atan:
    fld qword [esp + 4]
    fld1
    fpatan
    ret

atanf:
    fld dword [esp + 4]
    fld1
    fpatan
    ret

atanl:
    fld tword [esp + 4]
    fld1
    fpatan
    ret

atan2:
    fld qword [esp + 4]
    fld qword [esp + 12]
    fpatan
    ret

atan2f:
    fld dword [esp + 4]
    fld dword [esp + 8]
    fpatan
    ret

atan2l:
    fld tword [esp + 4]
    fld tword [esp + 16]
    fpatan
    ret

exp:
    fld qword [esp + 4]
    fldl2e
    fmul ST1
    f2xm1
    fld1
    fadd
    ret

expf:
    fld dword [esp + 4]
    fldl2e
    fmul ST1
    f2xm1
    fld1
    fadd
    ret

expl:
    fld tword [esp + 4]
    fldl2e
    fmul ST1
    f2xm1
    fld1
    fadd
    ret

; double pow(double base, double exp)
pow:
    fld1
    fld qword [esp + 4]
    fyl2x
    fld qword [esp + 12]
    fmulp
    f2xm1
    fld1
    faddp
    ret

powf:
    fld1
    fld dword [esp + 4]
    fyl2x
    fld dword [esp + 8]
    fmulp
    f2xm1
    fld1
    faddp
    ret
    
powl:
    fld1
    fld tword [esp + 4]
    fyl2x
    fld tword [esp + 16]
    fmulp
    f2xm1
    fld1
    faddp
    ret

log:
    fld1
    fld qword [esp + 4]
    fyl2x
    fldl2e
    fdivp ST1
    ret

logf:
    fld1
    fld dword [esp + 4]
    fyl2x
    fldl2e
    fdivp ST1
    ret

logl:
    fld1
    fld tword [esp + 4]
    fyl2x
    fldl2e
    fdivp ST1
    ret

log10:
    fld1
    fld qword [esp + 4]
    fyl2x
    fldl2t
    fdivp ST1
    ret

log10f:
    fld1
    fld dword [esp + 4]
    fyl2x
    fldl2t
    fdivp ST1
    ret

log10l:
    fld1
    fld tword [esp + 4]
    fyl2x
    fldl2t
    fdivp ST1
    ret

frexp:
    fld qword [esp + 4]
    fxtract		      ; mantissa = ST0, exponent = ST1
    fxch		      ; swap ST0,ST1
    mov dword eax, [esp + 12]
    fistp dword [eax]	      ; store ipart of ST0 in the int* at esp+12
    ret

frexpf:
    fld dword [esp + 4]
    fxtract
    fxch
    mov dword eax, [esp + 8]
    fistp dword [eax]
    ret

frexpl:
    fld tword [esp + 4]
    fxtract
    fxch
    mov dword eax, [esp + 16]
    fistp dword [eax]
    ret

ldexp:
    fild dword [esp + 12]
    fld qword [esp + 4]
    fscale
    ret

ldexpf:
    fild dword [esp + 8]
    fld dword [esp + 4]
    fscale
    ret

ldexpl:
    fild dword [esp + 16]
    fld tword [esp + 4]
    fscale
    ret

fabs:
    fld qword [esp + 4]
    fabs
    ret

fabsf:
    fld dword [esp + 4]
    fabs
    ret

fabsl:
    fld tword [esp + 4]
    fabs
    ret

modf:
    fld qword [esp + 4]
    fld1
    fld ST1
    .try:                    ; in case fprem needs to be called multiple times
    fprem
    fstsw ax
    test ax, 0000010000000000b ; test C2 status bit
    jnz .try
    fstp ST1
    fxch ST1
    fsub ST1
    mov dword eax, [esp + 12]
    fstp qword [eax]
    ret

modff:
    fld dword [esp + 4]
    fld1
    fld ST1
    .try:                    ; in case fprem needs to be called multiple times
    fprem
    fstsw ax
    test ax, 0000010000000000b ; test C2 status bit
    jnz .try
    fstp ST1
    fxch ST1
    fsub ST1
    mov dword eax, [esp + 8]
    fstp dword [eax]
    ret

modfl:
    fld tword [esp + 4]
    fld1
    fld ST1
    .try:                    ; in case fprem needs to be called multiple times
    fprem
    fstsw ax
    test ax, 0000010000000000b ; test C2 status bit
    jnz .try
    fstp ST1
    fxch ST1
    fsub ST1
    mov dword eax, [esp + 16]
    fstp tword [eax]
    ret

ceil:
    fld qword [esp + 4]
    sub esp, 2
    fstcw [esp]
    mov ax, [esp]
    and ax, 1111001111111111b
    or ax, 0000100000000000b
    mov [esp], ax
    fldcw [esp]
    frndint
    and ax, 1111001111111111b
    mov [esp], ax
    fldcw [esp]
    add esp, 2
    ret

ceilf:
    fld dword [esp + 4]
    sub esp, 2
    fstcw [esp]
    mov ax, [esp]
    and ax, 1111001111111111b
    or ax, 0000100000000000b
    mov [esp], ax
    fldcw [esp]
    frndint
    and ax, 1111001111111111b
    mov [esp], ax
    fldcw [esp]
    add esp, 2
    ret

ceill:
    fld tword [esp + 4]
    sub esp, 2
    fstcw [esp]
    mov ax, [esp]
    and ax, 1111001111111111b
    or ax, 0000100000000000b
    mov [esp], ax
    fldcw [esp]
    frndint
    and ax, 1111001111111111b
    mov [esp], ax
    fldcw [esp]
    add esp, 2
    ret

floor:
    fld qword [esp + 4]
    sub esp, 2
    fstcw [esp]
    mov ax, [esp]
    and ax, 1111001111111111b
    or ax, 0000010000000000b
    mov [esp], ax
    fldcw [esp]
    frndint
    and ax, 1111001111111111b
    mov [esp], ax
    fldcw [esp]
    add esp, 2
    ret    

floorf:
    fld dword [esp + 4]
    sub esp, 2
    fstcw [esp]
    mov ax, [esp]
    and ax, 1111001111111111b
    or ax, 0000010000000000b
    mov [esp], ax
    fldcw [esp]
    frndint
    and ax, 1111001111111111b
    mov [esp], ax
    fldcw [esp]
    add esp, 2
    ret

floorl:
    fld tword [esp + 4]
    sub esp, 2
    fstcw [esp]
    mov ax, [esp]
    and ax, 1111001111111111b
    or ax, 0000010000000000b
    mov [esp], ax
    fldcw [esp]
    frndint
    and ax, 1111001111111111b
    mov [esp], ax
    fldcw [esp]
    add esp, 2
    ret

fmod:
    fld qword [esp + 12]
    fld qword [esp + 4]
    .try:
    fprem
    fstsw ax
    test ax, 0000010000000000b
    jnz .try
    ret

fmodf:
    fld dword [esp + 8]
    fld dword [esp + 4]
    .try:
    fprem
    fstsw ax
    test ax, 0000010000000000b
    jnz .try
    ret

fmodl:
    fld tword [esp + 16]
    fld tword [esp + 4]
    .try:
    fprem
    fstsw ax
    test ax, 0000010000000000b
    jnz .try
    ret

section .data

floatformat: db '%f\n', 0
