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

SYSTEMCALL 0x0, getPID

SYSTEMCALL 0x3, screenGetColor
SYSTEMCALL 0x4, screenSetColor
SYSTEMCALL 0x5, screenPutChar
SYSTEMCALL 0x6, screenPutString
SYSTEMCALL 0x7, screenClear

SYSTEMCALL 0x8, keyboardReadReady
SYSTEMCALL 0x9, keyboardGetChar

SYSTEMCALL 0xa, malloc
SYSTEMCALL 0xb, free

SYSTEMCALL 0xc, PCIDirectScan

SYSTEMCALL 0xd, semctl