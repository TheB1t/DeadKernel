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
	stackTrace(20);
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
    	ELF32Obj_t module;
		for (uint32_t i = 0; i < mboot->mods_count; i++) {
			memset(&module, 0, sizeof(ELF32Obj_t));

        	if (!ELFLoad(modules[i].mod_start, &module))
            	continue;

			char* module_name = getModuleName(&module);

			if (module_name == NULL)
				continue;

			printf("%s (at 0x%08x) ", module_name, module.header);
		}

		printf("\n");
	}

	LOG_INFO("[GRUB] Loaded %s table",
		mboot->flags & MULTIBOOT_FLAG_AOUT	? "symbol" :
		mboot->flags & MULTIBOOT_FLAG_ELF	? "section" : "no one"
	);
	
	//init stacktrace variables
	if (mboot->flags & MULTIBOOT_FLAG_ELF) {
		initKernelTable((void*)mboot->addr, mboot->num, mboot->shndx);
	} else {
		WARN("Section table can't load! Stacktrace in semi-functional mode");
	}
	
    // for(uint32_t i = 0; i < mboot->mmap_length; i += sizeof(multiboot_memory_map_t)) {
    //     multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mboot->mmap_addr + i);
 
    //     printf("Start Addr: %08x | Length: %08x | Size: %04x | Status: %s\n",
    //         mmmt->addr_low, mmmt->len_low, mmmt->size, 
	// 		mmmt->type == MULTIBOOT_MEMORY_AVAILABLE ? "Available" :
	// 		mmmt->type == MULTIBOOT_MEMORY_RESERVED ? "Reserved" :
	// 		mmmt->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE ? "ACPI" :
	// 		mmmt->type == MULTIBOOT_MEMORY_NVS ? "NVS" :
	// 		mmmt->type == MULTIBOOT_MEMORY_BADRAM ? "BadRam" : "Unknown");
    // }

	return kernel_main();
}
