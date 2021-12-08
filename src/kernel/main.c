
#include "multiboot.h"
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"
#include "systimer.h"
#include "paging.h"
#include "kheap.h"
#include "task.h"
#include "syscall.h"
#include "pci.h"

extern uint32_t placementAddress;
uint32_t initialESP;

int main(multiboot_t* mboot, uint32_t initialStack) {
	initialESP = initialStack;
	
	initDescriptorTables();
	screenClear();
	
//	ASSERT(mboot->mods_count > 0);
	uint32_t initrdLocation = *((uint32_t*)mboot->mods_addr);
	uint32_t initrdEnd = *(uint32_t*)(mboot->mods_addr + 4);
	placementAddress = initrdEnd;
		
	initSysTimer(50);
	initPaging();

	if (BIOS32Find()) {
		printf("BIOS32 found on 0x%x\n", BIOS32GetAddress());
	} else {
		printf("BIOS32 not found\n");
	}
	
	uint8_t	HWMech, majorVer, minorVer;
	if (BIOS32CheckPCI(&majorVer, &minorVer, &HWMech)) {
		printf("PCI BIOS found %x %x:%x\n", HWMech, majorVer, minorVer);
	} else {
		printf("NOT FOUND\n");
	}

	printf("Scan PCI bus...\n");
	for (uint32_t i = 0; i < 0xFFFFFF; i++) {
		PCIDevice_t device;
		if (!PCIBIOSFindClass(i, 0, &device)) {
			printf("Found device %s [%x:%x:%x]\n", PCIGetClassName(i), device.bus, device.dev, device.fn);
		}
	}
	printf("Scan done!\n");
	initTasking();		
/*
	initSysCalls();
	switchToUserMode();

	syscall_screenPutString("Welcome to the fucking user-mode asshole!\n");
*/
	return 0xDEADBABA;
}
