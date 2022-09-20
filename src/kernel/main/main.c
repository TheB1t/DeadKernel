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
#include "stackTrace.h"

#include "elf.h"

extern uint32_t placementAddress;
extern int32_t kernel_main();
ELF32Header_t* testModule;

void memPrint(uint8_t* mem, uint32_t size) {
	#define OUT_W 16

	for (uint32_t i = 0; i < size / OUT_W; i++) {
		printf("%08X | ", OUT_W * i);
		for (uint32_t j = 0; j < OUT_W; j++) {
			printf("%02X ", mem[(OUT_W * i) + j]);
		}
		printf("| ");
		for (uint32_t j = 0; j < OUT_W; j++) {
			if (mem[(OUT_W * i) + j]  > 31)
				printf("%.1c", mem[(OUT_W * i) + j]);
			else
				printf(".");
		}
		printf("\n");
	}
}

int32_t main(multiboot_t* mboot) {	
	initDescriptorTables();
	screenClear();

	FPRINTF("[GRUB] Loaded %d modules\n", mboot->mods_count);
	multiboot_mods_t* mods = (multiboot_mods_t*)mboot->mods_addr;
	if (mboot->mods_count > 0) {
		testModule = (ELF32Header_t*)mods[0].mod_start;
		placementAddress = mods[mboot->mods_count - 1].mod_end;
	}

	uint8_t* t = "no one";
	t = (mboot->flags & MULTIBOOT_FLAG_AOUT) > 0 ? (uint8_t*)"symbol" : t;
	t = (mboot->flags & MULTIBOOT_FLAG_ELF) > 0 ? (uint8_t*)"section" : t;
	FPRINTF("[GRUB] Loaded %s table\n", t);
	
	//init stacktrace variables
	if (mboot->flags & MULTIBOOT_FLAG_ELF) {
		kernelSectionTable = (KernelSectionHeader_t*)mboot->addr;
		sectionTableSize = mboot->num;
		sectionStringTableIndex = mboot->shndx;
	} else {
		WARN("Section table can't load!");
	}
	
	return kernel_main();
}
