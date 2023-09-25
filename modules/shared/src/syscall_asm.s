%macro SYSTEMCALL 2
  [GLOBAL %2]
  type %2 function
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

SYSTEMCALL 0, getPID

SYSTEMCALL 1, screenGetColor
SYSTEMCALL 2, screenSetColor
SYSTEMCALL 3, screenPutChar
SYSTEMCALL 4, screenPutString
SYSTEMCALL 5, screenClear

SYSTEMCALL 6, keyboardReadReady
SYSTEMCALL 7, keyboardGetChar

SYSTEMCALL 8, malloc
SYSTEMCALL 9, free

SYSTEMCALL 10, PCIDirectScan

[GLOBAL yield]
yield:
	int 0x40
	ret