MAKEFLAGS += --no-print-directory

AUXFILES := Makefile

SRCFILES := $(shell find . -type f -name "*.c")
HDRFILES := $(shell find . -type f -name "*.h")
ASMFILES := $(shell find . -type f -name "*.s")

OBJEXISTS := $(shell find . -type f -name "*.o")
OBJFILES := $(patsubst %.s,%.o,$(ASMFILES)) $(patsubst %.c,%.o,$(SRCFILES))

ALLFILES := $(SRCFILES) $(HDRFILES) $(ASMFILES) $(AUXFILES)

CINCDIRS := $(shell find . -type d -exec echo "-I{}" \;)

EXEC 			:= kernel
IMAGE_PATH		:= ../hard_disk.img
MOUNT_PATH		:= ../image
BASIC_DIR		:= .

CC				:= gcc
ASM				:= nasm
LD				:= ld

CFLAGS			:= -m32 -g3 -fno-pic -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector
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
	-@rm $(EXEC)
	
link: $(OBJFILES)
	$(info [$(LD)]	Linking $(EXEC))
	@$(LD) $(LDFLAGS) -o $(EXEC) $(OBJFILES)

.s.o:
	$(info [$(ASM)]	Compiling $<)
	@$(ASM) $(ASFLAGS) -o $(patsubst %.s,%.o,$<) $<

.c.o: 
	$(info [$(CC)]	Compiling $<)
	@$(CC) -c $(CINCDIRS) $(CFLAGS) -o $(patsubst %.c,%.o,$<) $<

mountLoop:
	@sudo losetup -P /dev/loop100 $(IMAGE_PATH)

umountLoop:
	@sudo losetup -d /dev/loop100

mountImage: 
	@$(MAKE) mountLoop
	-@mkdir ${MOUNT_PATH}
	@sudo mount /dev/loop100p1 $(MOUNT_PATH)
	
umountImage:
	@sudo umount /dev/loop100p1
	-@rmdir ${MOUNT_PATH}
	@$(MAKE) umountLoop

pack: remake
	$(info [make]	Packing)
	@$(MAKE) mountImage
	-@sudo rm $(MOUNT_PATH)/$(BASIC_DIR)/$(EXEC)
	@sudo cp $(EXEC) $(MOUNT_PATH)/$(BASIC_DIR)
	@$(MAKE) umountImage

directRun: remake
	$(info [make]	Running directly)	
	-@sudo qemu-system-i386 -kernel $(EXEC)

run: pack
	$(info [make]	Running)
	@$(MAKE) mountLoop
	-@sudo qemu-system-i386 -hda /dev/loop100 -no-reboot -serial stdio
	@$(MAKE) umountLoop

runNoGraphic: pack
	$(info [make]	Running)
	@$(MAKE) mountLoop
	-@sudo qemu-system-i386 -hda /dev/loop100 -curses -serial mon:stdio
	@$(MAKE) umountLoop

debug: pack
	$(info [make]	Debug)
	@$(MAKE) mountLoop
	-@sudo qemu-system-i386 -s -S -hda /dev/loop100
	@$(MAKE) umountLoop
	
secInfo: clearConsole
	readelf -S $(EXEC)

symInfo: clearConsole
	readelf -s $(EXEC)
