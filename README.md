# DeadKernel

### About DeadKernel

DeadKernel will be a fully modular kernel in the future, almost every part of it will be presented as a module that can be replaced with another with more/less functionality, optimization, etc. But right now it's a simple core providing basic functionality for development, and nothing more.

### What tools i using for development

For writing code:
- Visual Studio Code
- [micro](https://github.com/zyedidia/micro?ysclid=l9jqlbouhf912724444)

For debugging kernel:
- qemu
- bochs
- gdb
- real hardware

### What implemented now

- Loading from Multiboot-compliant bootloader
- GDT, LDT, IDT cofiguration
- Simply timing support using PIT timer
- Basic support of paging mode
	- Frame/page alloc/free
	- Mirror allocating
	- Page directory cloning
	- Primitive page-fault handling
	- Page directory switching
- Simply memory manager
	- Alloc/free memory from heap
- Task sheduler
	- 6 task states
	- Creating task from ELF image
	- Switching between tasks have different privilege levels (with some errors)
	- Detecting task completion
	- Halt CPU if all tasks finished (includes kernel)
	- Yielding
- Simply PCI bus driver
- Basic screen output support
	- Half-functionally printf
- Stack trace support
	- If GRUB (or other Multiboot-compliant bootloader) loads section table, stacktrace printed with function names
- Basic syscalls support
- Serial port driver
- Keyboard driver
- ELF files reading

### What will be implemented in the future

- Task sheduling
	- Forking
- ACPI support
	- HPET timer
- Module loading
- All already implemented drivers as a module (including minimal drivers)
- Simple filesystem driver
- Simple initrd
- VESA driver
- Ethernet driver
- Network stack

### Requirements for build and run

```bash
# Install this packages (if you using debian-based system)
$ sudo apt -y install qemu qemu-system nasm gcc binutils make
```

### How to pack/run it

```bash
# Previous set part number for loop module
$ sudo modprobe -r loop
$ sudo modprobe loop max_part=10

# For pack module in image
$ cd src/modules/testModule
$ make pack
# Module automaticly will be packed to image

# For pack kernel in image
$ cd src/kernel
$ make pack

# For run kernel use
$ cd src/kernel
$ make run
# Kernel automaticly will be packed in image, and running using qemu
```

### How to contact me

__Email:__ bithovalsky@gmail.com
