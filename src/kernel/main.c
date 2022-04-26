#include "multiboot.h"
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"

uint32_t		initialESP;

extern uint32_t	placementAddress;
extern int32_t	kernel_main();

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
