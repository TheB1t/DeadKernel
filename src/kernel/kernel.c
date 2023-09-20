
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"
#include "systimer.h"
#include "paging.h"
#include "kheap.h"
#include "task.h"
#include "syscall.h"
#include "bios32.h"
#include "pci.h"
#include "pci_ids.h"

#include "keyboard.h"
#include "serial.h"

extern ELF32Header_t* testModule;

int32_t kernel_main() {
	initSysTimer();
	initPaging();

	//uint32_t d = *(uint32_t*)0xD0000000;
	
	if (BIOS32Find()) {
		LOG_INFO("BIOS32 found on 0x%08x", BIOS32GetAddress());
		
		uint8_t	HWMech, majorVer, minorVer;
		ASSERT(PCICheckSupport(&majorVer, &minorVer, &HWMech));
		LOG_INFO("PCI BIOS found %02x ver %02x.%02x", HWMech, majorVer, minorVer);
		
		LOG_INFO("Scan PCI bus...");
		PCIDevice_t devices[256];
		memset(devices, 0, sizeof(PCIDevice_t) * 256);

		uint32_t count = PCIDirectScan(devices);
		LOG_INFO("Found %d devices!", count);
		
		// PCIDevice_t* device = devices;
		// while (device->vendor) {
		// 	LOG_INFO("	%03d:%02d:%01d [%04x:%04x] %s", device->bus, device->dev, device->fn, device->vendor, device->device, PCIGetClassName(device->class));
		// 	device++;
		// }
	} else {
		LOG_INFO("BIOS32 instance not found!");
	}

	uint32_t entry = BIOS32GetService(*(uint32_t*)"$PIC");
	if (entry)
		LOG_INFO("Found");
	else
		LOG_INFO("Not found");
	serialInit(COM1, UART_BAUD_9600);

	initSysCalls();
	initTasking();
	
	DISABLE_INTERRUPTS();
	Task_t* t1 = makeTaskFromELF(testModule);
	runTask(t1);

	uint32_t pid = getPID();

	LOG_INFO("Message from kernel! PID %d Ring %d", pid, getCPL());
	ENABLE_INTERRUPTS();

	sleep(2000);

	LOG_INFO("Upime %d seconds", getUptime() / 1000);

	return 0xDEADBABA;
}