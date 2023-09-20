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
IMAGE_PATH		:= ../../hard_disk.img
MOUNT_PATH		:= ../../image
BASIC_DIR		:= modules


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
	@sudo losetup -P /dev/loop100 $(IMAGE_PATH)
	@sudo mount /dev/loop100p1 $(MOUNT_PATH)
	-@sudo rm $(MOUNT_PATH)/$(BASIC_DIR)/$(EXEC)
	@sudo cp $(EXEC) $(MOUNT_PATH)/$(BASIC_DIR)
	@sudo umount /dev/loop100p1
	@sudo losetup -d /dev/loop100
	
secInfo: clearConsole
	readelf -S $(EXEC)

symInfo: clearConsole
	readelf -s $(EXEC)
