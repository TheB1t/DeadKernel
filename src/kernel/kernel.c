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

int32_t kernel_main() {
	initSysTimer(50);
	initPaging();

	if (BIOS32Find()) {
		printf("BIOS32 found on 0x%08x\n", BIOS32GetAddress());
	} else {
		printf("BIOS32 not found\n");
	}
	
	uint8_t	HWMech, majorVer, minorVer;
	if (BIOS32CheckPCI(&majorVer, &minorVer, &HWMech)) {
		printf("PCI BIOS found %02x %02x:%02x\n", HWMech, majorVer, minorVer);
	} else {
		printf("NOT FOUND\n");
	}
	
	printf("Scan PCI bus...\n");
	PCIDevice_t devices[256];
	memset(devices, 0, sizeof(PCIDevice_t) * 256);

	PCIDirectScan(devices);

	PCIDevice_t* device = devices;
	while (device->vendor) {
		if (device->fn == 0) {
			printf("Bus %03d Device %02d Function %01d\n", device->bus, device->dev, device->fn);	
			printf("	Class  [%06x] %s\n",	device->class,	PCIGetClassName(device->class));
			printf("	Vendor [%04x] %s\n",	device->vendor,	PCIGetVendorName(device->vendor));
			printf("	Device [%04x]\n",		device->device);
		}
		device++;
	}
	
	printf("Scan done!\n");
	initTasking();		

	initSysCalls();
	switchToUserMode();

	syscall_screenPutString("Welcome to the fucking user-mode asshole!\n");
	return 0xDEADBABA;
}
