#include "multiboot.h"
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"
#include "systimer.h"
#include "paging.h"
#include "kheap.h"
#include "fs.h"
#include "initrd.h"

FSNode_t* ROOTFS;

extern uint32_t placementAddress;

int main(multiboot_t* mboot) {
	initDescriptorTables();
	screenClear();

	ASSERT(mboot->mods_count > 0);
	uint32_t initrdLocation = *((uint32_t*)mboot->mods_addr);
	uint32_t initrdEnd = *(uint32_t*)(mboot->mods_addr + 4);
	placementAddress = initrdEnd;

	initPaging();

	ROOTFS = initInitrd(initrdLocation);
	
	while (1) {
		
	}
	
	return 0xDEADBABA;
}
