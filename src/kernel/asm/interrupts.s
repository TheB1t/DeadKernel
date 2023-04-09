%macro ISR_NOERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push dword 0
		push dword %1
		jmp ASMInterruptPreHandler
%endmacro

%macro ISR_ERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push eax
		push dword %1
		jmp ASMInterruptPreHandler
%endmacro

%macro IRQ 2
	[GLOBAL irq%1]
	irq%1:
		cli
		push dword 0
		push dword %2
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

; typedef struct {
; 	uint32_t	ds;			;; - 30
; 	uint32_t	cr3;		;; - 26
; 	uint32_t	edi;		;; - 24
; 	uint32_t	esi;		;; - 20
; 	uint32_t	ebx;		;; - 16
; 	uint32_t	edx;		;; - 12
; 	uint32_t	ecx;		;; - 8
; 	uint32_t	eax;		;; - 4
; 	uint32_t	ebp;		;; - 0

;	WE ARE HERE
; 	uint32_t	err_code;	;; + 4
; 	uint32_t	int_no;		;; + 8

; 	// IRET Main
; 	uint32_t	eip;		;; + 12
; 	uint32_t	cs;			;; + 16
; 	uint32_t	eflags;		;; + 20

; 	// IRET Second
; 	uint32_t	esp0;		;; + 24
; 	uint32_t	ss0;		;; + 26
; } CPURegisters_t;

struc CPURegs
	._ds: resd 1
	._cr3: resd 1
	._edi: resd 1
	._esi: resd 1
	._ebx: resd 1
	._edx: resd 1
	._ecx: resd 1
	._eax: resd 1
	._ebp: resd 1

	._int_no: resd 1
	._err_code: resd 1
	
	; iret main
	._eip: resd 1
	._cs: resd 1
	._eflags: resd 1

	; iret second
	._esp0: resd 1
	._ss0: resd 1
endstruc

[EXTERN MainInterruptHandler]
ASMInterruptPreHandler:
    sub esp, 36
	
	mov [esp + CPURegs._eax], eax
	mov [esp + CPURegs._ebx], ebx
	mov [esp + CPURegs._ecx], ecx
	mov [esp + CPURegs._edx], edx
	mov [esp + CPURegs._esi], esi
	mov [esp + CPURegs._edi], edi
	mov [esp + CPURegs._ebp], ebp
	mov [esp + CPURegs._ds], ds
	mov eax, cr3
	mov [esp + CPURegs._cr3], eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push esp
    call MainInterruptHandler
    add esp, 4

	mov ax, [esp + CPURegs._ds]
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ebx, [esp + CPURegs._ebx]
	mov ecx, [esp + CPURegs._ecx]
	mov edx, [esp + CPURegs._edx]
	mov esi, [esp + CPURegs._esi]
	mov edi, [esp + CPURegs._edi]
	mov ebp, [esp + CPURegs._ebp]
	mov eax, [esp + CPURegs._cr3]
	mov cr3, eax
	mov eax, [esp + CPURegs._eax]

	add esp, 44 ; remove crap stuff

    ; xchg bx, bx
	sti
    iret