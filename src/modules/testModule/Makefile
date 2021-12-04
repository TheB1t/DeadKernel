AUXFILES := Makefile
PROJDIRS := ./

SRCFILES := $(shell find $(PROJDIRS) -type f -name "*.c")
HDRFILES := $(shell find $(PROJDIRS) -type f -name "*.h")
ASMFILES := $(shell find $(PROJDIRS) -type f -name "*.s")

OBJEXISTS := $(shell find $(PROJDIRS) -type f -name "*.o")
OBJFILES := $(patsubst %.s,%.o,$(ASMFILES)) $(patsubst %.c,%.o,$(SRCFILES))

ALLFILES := $(SRCFILES) $(HDRFILES) $(ASMFILES) $(AUXFILES)

CINCDIRS := $(foreach d,$(PROJDIRS),-I$(d))

EXEC 			:= testModule

CC				:= gcc
ASM				:= nasm
LD				:= ld

CFLAGS			:= -m32 -g -nostdlib -fno-pic -fno-pie -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS			:= -Tlink.ld -m elf_i386
ASFLAGS			:= -felf

all: clearConsole link
remake: clearConsole cleano link

clearConsole:
	clear

cleano:
	$(foreach r,$(OBJEXISTS),$(shell rm $(r)))
	$(info [make]	Removing *.o files)

clean: cleano
	$(info [make]	Clean)
	@rm $(EXEC)
	
link: $(OBJFILES)
	$(info [$(LD)]	Linking $(EXEC))
	@$(LD) $(LDFLAGS) -o $(EXEC) $(OBJFILES)

.c.o:
	$(info [$(CC)]	Compiling $<)
	@$(CC) -c $(CINCDIRS) $(CFLAGS) -o $(patsubst %.c,%.o,$<) $<
	
.s.o:
	$(info [$(ASM)]	Compiling $<)
	@$(ASM) $(ASFLAGS) -o $(patsubst %.s,%.o,$<) $<

pack: remake
	$(info [make]	Packing)
	-@mkdir image
	@sudo mount /dev/sdb1 image
	-@sudo rm image/modules/testModule
	@sudo cp testModule image/modules/testModule
	@sudo umount /dev/sdb1
	-@rm -rf image
	
secInfo: clearConsole
	readelf -S $(EXEC)

symInfo: clearConsole
	readelf -s $(EXEC)
