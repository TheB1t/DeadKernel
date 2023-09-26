; BIOS32_t* find_bios32(void)
[GLOBAL find_bios32]
type find_bios32 function
find_bios32:
    mov eax, 0x0E0000
    mov edx, 0x0FFFFF

check_loop:
    cmp eax, edx
    jg not_supported

    mov ebx, [eax]
    cmp ebx, '_32_'
    je supported

    add eax, 4
    jmp check_loop

not_supported:
    mov eax, 0
    ret

supported:
    ret