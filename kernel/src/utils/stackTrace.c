#include <utils/stackTrace.h>

ELF32Obj_t	kernelTableObj = { 0 };

void initKernelTable(void* sybtabPtr, uint32_t size, uint32_t shindex) {
	KERNEL_TABLE_OBJ->header = NULL;

	KERNEL_TABLE_OBJ->sections.sectab_head			= (ELF32SectionHeader_t*)sybtabPtr;
	KERNEL_TABLE_OBJ->sections.sectab_size			= size;

	KERNEL_TABLE_OBJ->sections.hstrtab_head			= (uint8_t*)ELF32_SECTION(KERNEL_TABLE_OBJ, shindex)->addr;

	ELF32SectionHeader_t* symtab					= ELFLookupSectionByType(KERNEL_TABLE_OBJ, ESHT_SYMTAB);
	KERNEL_TABLE_OBJ->sections.symtab_head			= symtab ? (ELF32Symbol_t*)symtab->addr : NULL;
	KERNEL_TABLE_OBJ->sections.symtab_size 			= symtab ? symtab->size / sizeof(ELF32Symbol_t) : 0;

	ELF32SectionHeader_t* strtab					= ELFLookupSectionByType(KERNEL_TABLE_OBJ, ESHT_STRTAB);
	KERNEL_TABLE_OBJ->sections.strtab_head			= strtab ? (uint8_t*)strtab->addr : NULL;
	KERNEL_TABLE_OBJ->sections.strtab_size			= strtab ? strtab->size : 0;

	ELF32Symbol_t* startsym							= ELFLookupSymbolByName(KERNEL_TABLE_OBJ, STT_NOTYPE, "__code");
	KERNEL_TABLE_OBJ->start							= startsym ? startsym->value : 0;

	ELF32Symbol_t* endsym							= ELFLookupSymbolByName(KERNEL_TABLE_OBJ, STT_NOTYPE, "__end");
	KERNEL_TABLE_OBJ->end							= endsym ? endsym->value : 0;
}

uint8_t isKernelSectionTableLoaded() {
	return ELF32_TABLE_SIZE(KERNEL_TABLE_OBJ, sec) > 0 && ELF32_TABLE(KERNEL_TABLE_OBJ, hstr) != NULL;
}

typedef struct stackFrame {
	struct stackFrame* ebp;
	uint32_t eip;
} stackFrame_t;

void printStackFrame(uint32_t address, bool ifFaulting) {
	if (!address)
		return;

	CPURegisters_t* context = getInterruptedContext();
	ELF32Obj_t* hdr = KERNEL_TABLE_OBJ;
	ELF32Symbol_t* symbol = NULL;

	while (true) {
		symbol = ELFGetNearestSymbolByAddress(hdr, STT_FUNC, address);

		if (!symbol && isTaskingInit() && hdr != currentTask->elf_obj) {
			hdr = currentTask->elf_obj;
			continue;
		}

		break;
	}

	if (context) {
		static bool printed = true;
		if (context->eip > hdr->start && context->eip < hdr->end && address > hdr->start && address < hdr->end && !ifFaulting) {
			if (!printed) {
				printStackFrame(context->eip, true);
				printed = true;
			}
		} else {
			printed = false;
		}
	}


	printf("%-3s ", ifFaulting ? "-->" : "");

    if (symbol) {
		uint8_t* name = hdr->sections.strtab_head + symbol->name;
		
		printf("0x%08x -> %-25s at address 0x%08x ", address, name, symbol->value);
	} else {
		printf("0x%08x -> %-25s at address 0x%08x ", address, "UNKNOWN", address);
	}

	if (isTaskingInit()) {
		if (symbol && hdr == currentTask->elf_obj) {
			char* name = getModuleName(currentTask->elf_obj);
			printf("in %s ", name);
		} else {
			printf("in kernel ");
		}
	}

	printf("\n");
}

void stackTrace(uint32_t maxFrames) {
    printf("--[[						Stack trace begin						]]--\n");

	stackFrame_t* stk;
	__asm__ volatile("movl %%ebp, %0" : "=r"(stk));
    
    for(; maxFrames > 0; maxFrames--) {
		if (!stk) break;
		printStackFrame(stk->eip, false);
        stk = stk->ebp;
    }

    printf("--[[						Stack trace end			 				]]--\n");
}
