%include "io.inc"
section .rodata align = 4
L0:
    dd L1 ;c = 5
    dd L2 ;c = 6
    dd L3 ;c = 7
    dd L4 ;c = 8
    

section .bss
    c resd 1
    x resd 1
    y resd 1
    
section .text
global main
main:
    GET_DEC 4, eax
    mov [c], eax
    GET_DEC 4, eax
    mov [x], eax
    GET_DEC 4, eax
    mov [y], eax
    mov eax, dword[c]
    sub eax, 5    
    jmp [L0 + 4*eax]
L1:
    mov eax, dword[x]
    add eax, dword[y]
    PRINT_DEC 4, eax
    xor eax, eax
    ret
L2:
    mov eax, dword[x]
    sub eax, dword[y]
    PRINT_DEC 4, eax
    xor eax, eax
    ret
L3:
    mov eax, dword[x]
    imul dword[y]
    PRINT_DEC 4, eax
    xor eax, eax
    ret
L4:
    mov eax, dword[x]
    cdq
    idiv dword[y]
    PRINT_DEC 4, eax
    xor eax, eax
    ret

    