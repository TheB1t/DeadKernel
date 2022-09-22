%macro ISR_NOERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		mov [temp_err_code], dword 0
		mov [temp_int_no], dword %1
		jmp ASMInterruptPreHandler
%endmacro

%macro ISR_ERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push eax
		mov eax, [esp + 4]
		pop eax
		add esp, 4
		mov [temp_err_code], eax
		mov [temp_int_no], dword %1
		jmp ASMInterruptPreHandler
%endmacro

%macro IRQ 2
	[GLOBAL irq%1]
	irq%1:
		cli
		mov [temp_err_code], dword 0
		mov [temp_int_no], dword %2
		jmp ASMInterruptPreHandler
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

ISR_NOERRCODE 64
ISR_NOERRCODE 128

IRQ		0, 32
IRQ		1, 33
IRQ		2, 34
IRQ		3, 35
IRQ		4, 36
IRQ		5, 37
IRQ		6, 38
IRQ		7, 39
IRQ		8, 40
IRQ		9, 41
IRQ		10, 42
IRQ		11, 43
IRQ		12, 44
IRQ		13, 45
IRQ		14, 46
IRQ		15, 47

[EXTERN MainInterruptHandler]
ASMInterruptPreHandler:
	push esp ;28 ;32
	push ebp ;24 ;28	
	push eax ;20 ;24
	push ecx ;16 ;20
	push edx ;12 ;16
	push ebx ;8  ;12
	push esi ;4  ;8
	push edi ;0  ;4
	mov eax, cr3
	push eax	 ;0

	;Restoring the ESP to the pre-interrupt state (this value will get into the high-level handler)
	mov eax, [esp + 32]
	add eax, 12
	mov [esp + 32], eax

	xor eax, eax
	mov ax, ss
	and eax, 3
	mov [.cpl], eax

	xor eax, eax
	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	push esp

	mov eax, [temp_err_code]
	push eax
	mov eax, [temp_int_no]
	push eax

	call MainInterruptHandler
	
	add esp, 12

	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	mov [.ss], bx

	;Copy IRET context
	mov ebx, [esp + 44] ;EFLAGS
	mov [.eflags], ebx;
	
	mov ebx, [esp + 40] ;CS
	mov [.cs], ebx;
	
	mov ebx, [esp + 36] ;EIP
	mov [.eip], ebx;

	pop eax
	mov [.cr3], eax

	pop edi
	pop esi
	pop ebx
	pop edx
	pop ecx
	pop eax
	pop ebp
	pop esp

	mov [.tmp], eax

	;Change page dir
	mov eax, [.cr3]
	mov cr3, eax

	;Build IRET context
	sub esp, 12

	xor eax, eax
	mov eax, [.cs]
	and eax, 3
	cmp eax, [.cpl]
	jz .notChangeRing1

	mov ax, ss
	cmp ax, 0x28
	jz .notChangeRing1
	sub esp, 8
	
	mov eax, [.ss]
	mov [esp + 16], eax ;SS

	mov eax, esp
	add eax, 20
	mov [esp + 12], eax ;ESP

.notChangeRing1:

	mov eax, [.eflags] ;EFLAGS
	mov [esp + 8], eax;
	
	mov eax, [.cs] ;CS
	mov [esp + 4], eax;
	
	mov eax, [.eip] ;EIP
	mov [esp], eax;

	mov eax, [.tmp]

	;xchg bx, bx

	sti
	iret

.eflags: dd 0
.cs:	dd 0
.ss:	dd 0
.cpl:	dd 0
.eip:	dd 0
.cr3:	dd 0
.tmp:	dd 0

temp_int_no: dd 0
temp_err_code: dd 0