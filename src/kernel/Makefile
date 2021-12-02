SOURCES = boot.o main.o common.o screen.o descriptor_tables.o descriptor_tables_asm.o interrupts.o isr.o systimer.o kheap.o paging.o

CFLAGS = -m32 -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS = -Tlink.ld -m elf_i386
ASFLAGS = -felf

all: $(SOURCES) link

clean:
	rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.c.o:
	gcc -c $(CFLAGS) $<
	
.s.o:
	nasm $(ASFLAGS) $<
