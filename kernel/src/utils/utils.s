[GLOBAL __interruptsDisable]
[GLOBAL __disableInterrupts]
type __disableInterrupts function
__disableInterrupts:
    cli
    
    lock inc dword [__interruptsDisable]

    push eax

    xor eax, eax
    mov eax, [__interruptsDisable]
    cmp eax, 0
    jnz .continue

	lock dec dword [__interruptsDisable]

    sti
.continue:
    pop eax
    ret

[GLOBAL __enableInterrupts]
type __enableInterrupts function
__enableInterrupts:
    cli
    
    push eax

    lock dec dword [__interruptsDisable]

    xor eax, eax
    mov eax, [__interruptsDisable]
    cmp eax, 0
    jnz .continue
    sti

.continue:
    pop eax
    ret

section .data
    __interruptsDisable: dd 0