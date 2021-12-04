[GLOBAL copyPagePhysical]
copyPagePhysical:
	push ebx
	pushf

	cli

	mov ebx, [esp + 12]
	mov ecx, [esp + 16]

	mov edx, cr0
	and edx, 0x7FFFFFFF
	mov cr0, edx

	mov edx, 1024

.loop:
	mov eax, [ebx]
	mov [ecx], eax
	add ebx, 4
	add ecx, 4
	dec edx
	jnz .loop

	mov edx, cr0
	or	edx, 0x80000000
	mov cr0, edx

	sti
	
	popf
	pop ebx	
	ret
	
[EXTERN taskHalted]
[GLOBAL loadEntry]
loadEntry:
	sub esp, 8
	
	xor ebp, ebp
	push ebp
	mov ebp, esp

	call eax
	push eax
	call taskHalted
	jmp $
