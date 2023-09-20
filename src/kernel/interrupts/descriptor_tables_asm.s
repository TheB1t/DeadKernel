[GLOBAL GDTFlush]
GDTFlush:
	mov eax, [esp + 4]
	lgdt [eax]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp 0x08:.flush
.flush:
	ret

[GLOBAL IDTFlush]
IDTFlush:
	mov eax, [esp + 4]
	lidt [eax]
	ret

[GLOBAL TSSFlush]
TSSFlush:
	xor eax, eax
	mov eax, [esp + 4]
	or ax, 3
	ltr ax
	ret