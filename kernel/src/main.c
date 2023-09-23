#include <multiboot.h>
#include <utils/common.h>
#include <interrupts/descriptor_tables.h>
#include <utils/stackTrace.h>
#include <modules/modules.h>

#include <fs/elf/elf.h>

extern uint32_t placementAddress;
extern int32_t kernel_main();

typedef struct {
	uint32_t	E			:1;
	uint32_t	TBL			:2;
	uint32_t	index		:13;
	uint32_t	RESERVED	:16;
} SelectorErrorCode_t;

void GPFHandler(CPURegisters_t* regs) {
	if (regs->err_code) {
		SelectorErrorCode_t* sec = (SelectorErrorCode_t*)regs->err_code;
		printf("[%s GPF] %s 0x%08x at address 0x%08x\n",
			sec->E ? "External" : "Internal",
			sec->TBL == 0 ? "GDT" :
			sec->TBL == 1 ? "IDT" :
			sec->TBL == 2 ? "LDT" : "IDT",
			sec->index,
			regs->eip
		);
	}
	//stackTrace(6);
	//BREAKPOINT;
	for(;;);
}

int32_t main(multiboot_t* mboot) {	
	initDescriptorTables();
	registerInterruptHandler(13, GPFHandler);
	screenClear();


	LOG_INFO("[GRUB] Loaded %d modules", mboot->mods_count);
	multiboot_mods_t* modules = (multiboot_mods_t*)mboot->mods_addr;
	initModules(modules, mboot->mods_count);

	if (mboot->mods_count > 0) {
		placementAddress = modules[mboot->mods_count - 1].mod_end;

		printf("Loaded modules: ");
		for (uint32_t i = 0; i < mboot->mods_count; i++) {
			ELF32Header_t* module = (ELF32Header_t*)modules[i].mod_start;
			char* module_name = getModuleName(module);

			if (module_name == NULL)
				continue;

			printf("%s ", module_name);
		}

		printf("\n");
	}

	LOG_INFO("[GRUB] Loaded %s table",
		mboot->flags & MULTIBOOT_FLAG_AOUT	? "symbol" :
		mboot->flags & MULTIBOOT_FLAG_ELF	? "section" : "no one"
	);
	
	//init stacktrace variables
	if (mboot->flags & MULTIBOOT_FLAG_ELF) {
		kernelSectionTable = (KernelSectionHeader_t*)mboot->addr;
		sectionTableSize = mboot->num;
		sectionStringTableIndex = mboot->shndx;
	} else {
		WARN("Section table can't load! Stacktrace in semi-functional mode");
	}
	
	return kernel_main();
}
