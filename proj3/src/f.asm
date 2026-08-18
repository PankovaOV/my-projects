
section .rodata
    c1 dq 2.0
    c2 dq 1.0
    c3 dq -2.0
    c4 dq 3.0
section .text
global f_1

f_1:
    push ebp
    mov ebp, esp
    
    mov esi, 0
    
    finit
    fld qword[ebp + 8]
    fldz
    fucomip
    jc .continue
    mov esi, 1
    fabs
    
    .continue:
    fldl2e
    fmulp
    
    mov ecx, 0
    .loop:
    fld1
    fucomip 
    jnc .ex
    fld qword[c1]
    fdivp
    inc ecx
    jmp .loop
    
    
    .ex:
    cmp esi, 0
    je .pass
    fchs
    
    .pass:
    f2xm1
    fld1
    faddp
    
    .loop1:
    cmp ecx, 0
    je .exit
    fld st0
    fmulp
    dec ecx
    jmp .loop1

    
    .exit:
    fld qword[c1]
    faddp
    mov esp, ebp
    pop ebp
    ret
    
global f_2
f_2:
    push ebp
    mov ebp, esp
    mov esi, 0
    
    finit
    fld qword[c2]
    fld qword[ebp + 8]
    
    fdivp 
    fchs

    
    .exit:
    
    mov esp, ebp
    pop ebp
    ret

global f_3
f_3:
    push ebp
    mov ebp, esp
    finit
    fld qword[ebp + 8]
    fld1
    faddp
    fld qword[c3]
    fmulp
    fld qword[c4]
    fdivp
    mov esp, ebp
    pop ebp
    ret