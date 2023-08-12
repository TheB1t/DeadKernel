# DeadKernel

__Version:__ alpha-0.2

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

For building:
- cmake

### What has been implemented now and what is planned

- [x] Loading from Multiboot-compliant bootloader
- [x] `GDT`, `LDT`, `IDT` cofiguration
- [x] Simply timing support using `PIT timer`
- [x] Basic support of paging mode
	- [x] Allocation/freeing of physical frames
	- [x] Allocation/freeing of pages
	- [x] Mirror allocating
	- [x] Page directory cloning
	- [x] Page directory freeing
	- [x] Primitive page-fault handling
	- [x] Page directory switching
- [x] Simply memory manager
	- [x] Alloc/free memory from kernel heap
- [x] Task sheduler
	- [x] 7 task states
	- [x] Creating task from ELF image
	- [x] Switching between tasks have different privilege levels
	- [x] Detecting task completion
	- [x] Halt CPU if all tasks finished (includes kernel)
	- [x] Yielding
	- [ ] Forking
- [x] Simply PCI bus driver
- [x] Basic screen output support
	- [x] Half-functionally printf
- [x] Stack trace support
	- [x] If GRUB (or other Multiboot-compliant bootloader) loads section table, stacktrace printed with function names
- [x] Basic syscalls support
- [x] Serial port driver
- [x] Keyboard driver
- [x] ELF files reading
- [ ] ACPI support
	- [ ] HPET timer
- [ ] Module loading
- [ ] All already implemented drivers as a module (including minimal drivers)
- [ ] Simple filesystem driver
- [ ] Simple initrd
- [ ] VESA driver
- [ ] Ethernet driver
- [ ] Network stack

### Requirements for build and run

1. Install this packages (if you using debian-based system):
```bash
sudo apt -y install qemu qemu-system nasm gcc binutils make cmake
```

### How to build/pack/run it

1. Build and pack kernel into `disk.img`:
```bash
./utils.sh pack
```

2. Run using `qemu`:
```bash
sudo qemu-system-x86_64 -hda disk.img
```

or you can run the build manually using:
```bash
cmake -B build .
make -C build
```
the compiled binaries will be located in the `build/bin` folder.

### How to contact me

__Email:__ bithovalsky@gmail.com