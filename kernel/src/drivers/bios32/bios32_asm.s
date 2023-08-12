; BIOS32_t* find_bios32(void)
[GLOBAL find_bios32]
find_bios32:
    mov eax, 0x0E0000    ; начало диапазона адресов BIOS
    mov edx, 0x0FFFFF    ; конец диапазона адресов BIOS

check_loop:
    cmp eax, edx     ; проверяем, что адрес не выходит за диапазон BIOS
    jg not_supported ; если вышли за диапазон - BIOS32 не поддерживается

    mov ebx, [eax]    ; загружаем значение по адресу в регистр ebx
    cmp ebx, '_32_'   ; сравниваем значение с маркером BIOS32
    je supported      ; если значения совпадают - BIOS32 поддерживается

    add eax, 4        ; увеличиваем адрес на 4 байта
    jmp check_loop    ; повторяем проверку

not_supported:
    mov eax, 0
    ret

supported:
    ret