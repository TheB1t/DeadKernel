
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"
#include "systimer.h"
#include "paging.h"
#include "kheap.h"
#include "task.h"
#include "syscall.h"
#include "pci.h"
#include "pci_ids.h"

#include "keyboard.h"

extern ELF32Header_t* testModule;

int32_t kernel_main() {
	initSysTimer(50);
	initPaging();

	//uint32_t d = *(uint32_t*)0xD0000000;
	
	if (BIOS32Find()) {
		FPRINTF("BIOS32 found on 0x%08x\n", BIOS32GetAddress());
		
		uint8_t	HWMech, majorVer, minorVer;
		ASSERT(BIOS32CheckPCI(&majorVer, &minorVer, &HWMech));
		FPRINTF("PCI BIOS found %02x ver %02x.%02x\n", HWMech, majorVer, minorVer);
		
		FPRINTF("Scan PCI bus...");
		PCIDevice_t devices[256];
		memset(devices, 0, sizeof(PCIDevice_t) * 256);

		uint32_t count = PCIDirectScan(devices);
		printf("%d devices!\n", count);
		
		PCIDevice_t* device = devices;
		while (device->vendor) {
			FPRINTF("	%03d:%02d:%01d [%04x:%04x] %s\n", device->bus, device->dev, device->fn, device->vendor, device->device, PCIGetClassName(device->class));
			device++;
		}
	} else {
		FPRINTF("BIOS32 instance not found!\n");
	}


	initSysCalls();
	initTasking();
	
	Task_t* t1 = makeTaskFromELF(testModule);
	runTask(t1);

	Task_t* t2 = makeTaskFromELF(testModule);
	runTask(t2);
	//runTask(t1);
	//freeTask(t1);
//	uint32_t id = fork();
//	yield();
	printf("Message from kernel! PID %d\n", getPID());


	while (1) {
		//yield();
		//printf("1");
	}

//	switchToUserMode();
	
	//syscall_screenClear();
//	syscall_screenPutString("Message from user mode!!\n");
	return 0xDEADBABA;
}

void kernel_halt() {
	uint8_t good = 0x02;
	while (good & 0x02)
		good = inb(0x64);
	outb(0x64, 0xFE);
loop:
	asm volatile("hlt");
	goto loop;
}
