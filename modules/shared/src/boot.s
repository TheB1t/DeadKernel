[EXTERN main]
[GLOBAL start]
type start function
start:
	call main
	mov ecx, 0xFE11DEAD
	jmp $