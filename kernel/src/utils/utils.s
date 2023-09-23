[EXTERN __interruptsDisable]

[GLOBAL __disableInterrupts]
__disableInterrupts:
    mov eax, [__interruptsDisable]

    cmp eax, 0
    jnz .continue
    cli

	inc eax
	mov [__interruptsDisable], eax

.continue:
    ret

[GLOBAL __enableInterrupts]
__enableInterrupts:
    mov eax, [__interruptsDisable]
	dec eax
	mov [__interruptsDisable], eax

    cmp eax, 0
    jnz .continue
    sti

.continue:
    ret