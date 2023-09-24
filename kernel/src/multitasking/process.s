[EXTERN __enableInterrupts]
[EXTERN __disableInterrupts]

[GLOBAL copyPagePhysical]
type copyPagePhysical function
copyPagePhysical:
	push ebx
	pushf

	call __disableInterrupts

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

	call __enableInterrupts
	
	popf
	pop ebx	
	ret