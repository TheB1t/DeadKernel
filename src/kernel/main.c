#include "multiboot.h"
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"
#include "systimer.h"
#include "paging.h"
#include "kheap.h"
#include "task.h"
#include "syscall.h"

extern uint32_t placementAddress;
uint32_t initialESP;

int main(multiboot_t* mboot, uint32_t initialStack) {
	initialESP = initialStack;
	
	initDescriptorTables();
	screenClear();

	initSysTimer(50);
	
//	ASSERT(mboot->mods_count > 0);
	uint32_t initrdLocation = *((uint32_t*)mboot->mods_addr);
	uint32_t initrdEnd = *(uint32_t*)(mboot->mods_addr + 4);
	placementAddress = initrdEnd;
	
	initPaging();
	initTasking();
	initSysCalls();
	switchToUserMode();

//	printf("OOPS, ERROR!\n");
	syscall_screenPutString("Welcome to the fucking user-mode asshole!\n");

	return 0xDEADBABA;
}
