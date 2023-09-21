[GLOBAL vesaSet]
vesaSet:
    mov ax, [esp + 4]
    mov bx, [esp + 8]
    int 0x10
    cmp ax, 0x004F
    jne error
    
    mov eax, 1
    ret

error:
    mov eax, 0
    ret