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
extern int32_t kernel_main();
uint32_t initialESP;

int32_t main(multiboot_t* mboot, uint32_t initialStack) {
	initialESP = initialStack;
	
	initDescriptorTables();
	screenClear();
	
//	ASSERT(mboot->mods_count > 0);
	uint32_t initrdLocation	= *(uint32_t*)mboot->mods_addr;
	uint32_t initrdEnd		= *(uint32_t*)(mboot->mods_addr + 4);
	placementAddress = initrdEnd;

	return kernel_main();
}
