MBOOT_PAGE_ALIGN    equ 1<<1
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

[GLOBAL mboot]
[EXTERN code]
[EXTERN bss]
[EXTERN end]

mboot:
  dd  MBOOT_HEADER_MAGIC
  dd  MBOOT_HEADER_FLAGS
  dd  MBOOT_CHECKSUM
   
  dd  mboot
  dd  code
  dd  bss
  dd  end
  dd  start

[GLOBAL start]
[GLOBAL kernelHaltedLabel]
[EXTERN main]
type start function
start:
  push    ebx
  ; Execute the kernel:
  cli
  xor ebp, ebp
  call main
  
  mov ecx, 0xFE11DEAD
  mov ebx, 0x0F000000
.preHaltLoop:
  sub ebx, 1
  jnz .preHaltLoop

  cli

  xor eax, eax
  mov al, 0x02
  mov dx, 0x64
.goodLoop:
  in al, dx
  and al, 0x02
  jnz .goodLoop
  
  mov al, 0xFE
  out dx, al

.halt:
  hlt
  jmp .halt

  