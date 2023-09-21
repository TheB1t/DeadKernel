%macro SYSTEMCALL 2
  [GLOBAL %2]
  %2:

  	mov eax, %1
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	mov esi, [esp + 16]	
	mov edi, [esp + 20]
	int 0x80
	ret
%endmacro

SYSTEMCALL 0, screenGetColor
SYSTEMCALL 1, screenSetColor
SYSTEMCALL 2, screenPutChar
SYSTEMCALL 3, screenPutString
SYSTEMCALL 4, screenClear

SYSTEMCALL 5, getPID

[GLOBAL yield]
yield:
	int 0x40
	ret