
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
	uint32_t tmpDword = 0;
	for (uint32_t bus = 0; bus < 256; bus++) {
		for (uint32_t dev = 0; dev < 32; dev++) {
			for (uint32_t fn = 0; fn < 8; fn++) {
				PCIDirectRead(bus, dev, fn, 0x00, 4, &tmpDword);
				uint16_t deviceID = (tmpDword >> 16)	& 0xFFFF;
				uint16_t vendorID = (tmpDword >> 0)		& 0xFFFF; 
				if (vendorID != 0xFFFF) {
					PCIDirectRead(bus, dev, 0, 0x08, 4, &tmpDword);
					uint32_t classCode = tmpDword >> 8;
					
					printf("%03d:%02d:%01d -> [%04x:%04x] %s\n", bus, dev, fn, vendorID, deviceID, PCIGetClassName(classCode));			
				}
			}
		}
	}
	printf("Scan done!\n");
	initTasking();		

	initSysCalls();
	switchToUserMode();

	syscall_screenPutString("Welcome to the fucking user-mode asshole!\n");
	return 0xDEADBABA;
}
