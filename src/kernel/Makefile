MAIN			= boot.o main.o kernel.o
INTERRUPTS		= descriptor_tables.o descriptor_tables_asm.o interrupts.o isr.o
UTILS			= common.o common_asm.o screen.o
MEM_MANAGMENT	= kheap.o paging.o
SYS_CLOCKS		= systimer.o
TASKING			= process.o task.o syscall.o
HW_SUPPORT		= pci.o
SOURCES			= $(MAIN) $(INTERRUPTS) $(UTILS) $(MEM_MANAGMENT) $(SYS_CLOCKS) $(TASKING) $(HW_SUPPORT)

CFLAGS			= -m32 -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS			= -Tlink.ld -m elf_i386
ASFLAGS			= -felf

all: $(SOURCES) link

clean:
	rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.c.o:
	gcc -c $(CFLAGS) $<
	
.s.o:
	nasm $(ASFLAGS) $<
