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

#### Hardware list

| Manufacturer | Model | Type        | Chipset   | Processor               | RAM                                 |
| ------------ | ----- | ----------- | --------- | ----------------------- | ----------------------------------- |
| HP           | T5570 | Thin Client | VIA VX900 | VIA Nano U3500, 1000MHz | `1Gb Samsung M471B2873FHS-CH9` x1 |


### What has been implemented now and what is planned

- [x] Loading from Multiboot-compliant bootloader
- [x] Support of `GDT`, `LDT` and `IDT` cofiguration
- [x] Simply timing support using `PIT timer`
- [x] Basic support of paging mode
	- [x] Allocation/freeing of physical frames
	- [x] Allocation/freeing of pages
	- [x] Mirror allocating
	- [x] Page directory cloning
	- [x] Page directory freeing
	- [x] Primitive page-fault handling
		- [x] Automatic page allocation if the page is not present
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
- [x] Simply `PCI` bus driver
- [x] Basic screen output support
	- [x] Half-functionally printf
- [x] Stack trace support
	- [x] If GRUB (or other Multiboot-compliant bootloader) loads section table, stacktrace printed with function names
- [x] Basic syscalls support
- [x] Serial port driver
- [x] Keyboard driver
- [x] ELF files reading
- [x] ACPI support (half)
	- [ ] HPET timer
- [x] Module loading
	- [x] Adding information to a module at compile time for identification by the kernel
- [ ] All already implemented drivers as a module (including minimal drivers)
- [ ] Simple filesystem driver
- [ ] Simple initrd
- [ ] VESA driver
- [ ] Ethernet driver
- [ ] Network stack

### Requirements for build and run

1. Install this packages (if you using debian-based system):
```bash
sudo apt -y install qemu qemu-system nasm gcc binutils make cmake grub-pc
```

### How to build/pack/run it

1. Create image
```bash
./utils.sh init
```

2. Build and pack kernel into image:
```bash
./utils.sh pack
```

2. Run using `qemu`:
```bash
./utils.sh run
```

or you can run the build manually using:
```bash
cmake -B build .
make -C build
```
the compiled binaries will be located in the `build/bin` folder.

### How to run it on hardware

##### PXE (Preboot eXecution Environment) way

*Note: Basically, to run on hardware, I use this method if the hardware supports PXE, simply because it is faster.*

1. Install `tftpd-hpa`, `isc-dhcp-server` and `grub-pc`
```bash
sudo apt -y install sudo apt install tftpd-hpa isc-dhcp-server grub-pc
```

2. Configure tftp (/etc/default/tftpd-hpa)
```
TFTP_USERNAME="tftp"
TFTP_ADDRESS="0.0.0.0:69"
TFTP_OPTIONS="-vvvv --ipv4 -s /var/lib/tftpboot"
```

3. Create grub network directory
```bash
sudo grub-mknetdir --net-directory /var/lib/tftpboot/
```

4. Configure dhcp (/etc/dhcp/dhcpd.conf)
```
subnet 192.168.50.0 netmask 255.255.255.0 {
  range 192.168.50.100 192.168.50.200;
  option routers 192.168.50.1;
  next-server 192.168.50.1;
  filename "boot/grub/i386-pc/core.0";
}
```

5. Configure the network interface to which hardware is connected
```
IP: 192.168.50.1
MASK: 255.255.255.0
GATEWAY: None
```

6. Restart tftp and dhcp daemons
```bash
sudo systemctl restart isc-dhcp-server.service
sudo systemctl restart tftpd-hpa.service
```

7. Compile kernel
```bash
./utils.sh pack_pxe
```

8. Initiate PXE boot on hardware

##### USB way

* Will be soon

### How to contact me

__Email:__ bithovalsky@gmail.com