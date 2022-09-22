
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
#include "serial.h"

extern ELF32Header_t* testModule;

int32_t kernel_main() {
	initSysTimer(5000);
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


	serialInit(COM1, UART_BAUD_9600);

	initSysCalls();
	initTasking();
	
	Task_t* t1 = makeTaskFromELF(testModule, 1);
	runTask(t1);

	//Task_t* t2 = makeTaskFromELF(testModule, 1);
	//runTask(t2);


	uint32_t pid = getPID();

	yield();

	DISABLE_INTERRUPTS;
	printf("Message from kernel! PID %d Ring %d\n", pid, getCPL());
	ENABLE_INTERRUPTS;


	for (uint32_t i = 0; i < 0x100; i++) {
		yield();
	};

	return 0xDEADBABA;
}