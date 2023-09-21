#include <utils/stackTrace.h>

KernelSectionHeader_t* kernelSectionTable = 0;
uint32_t sectionStringTableIndex = 0;
uint32_t sectionTableSize = 0;

uint8_t isKernelSectionTableLoaded() {
	return kernelSectionTable != NULL;
}

KernelSectionHeader_t* kernelSection(uint32_t idx) {
	return &kernelSectionTable[idx];
}

uint8_t* kernelLookupString(uint32_t offset) {
	return (uint8_t*)kernelSection(sectionStringTableIndex)->addr + offset;
}

KernelSectionHeader_t* kernelLookupSectionByName(char* name) {
	if (!isKernelSectionTableLoaded())
		return NULL;
		
	for (uint32_t i = 0; i < sectionTableSize; i++) {
		KernelSectionHeader_t* section = kernelSection(i);
		if (strcmp(kernelLookupString(section->name), name))
			continue;

		return section; 
	}
	
	return NULL;
}

uint8_t* kernelGetSymbolNameByAddress(int32_t address) {
	if (address == 0)
		return SYMBOL_NOT_FOUND;
		
	KernelSectionHeader_t* strtab_section = kernelLookupSectionByName(".strtab");
	if (strtab_section == NULL)
		return SYMBOL_NOT_FOUND;

	uint8_t* strtab = (uint8_t*)strtab_section->addr;
	
	KernelSectionHeader_t* symtab_section = kernelLookupSectionByName(".symtab");
	if (symtab_section == NULL)
		return SYMBOL_NOT_FOUND;
		
	uint32_t symtab_num = symtab_section->size / sizeof(KernelSymbol_t);
	KernelSymbol_t* symtab = (KernelSymbol_t*)symtab_section->addr;

	for (uint32_t i = 0; i < symtab_num; i++) {
		KernelSymbol_t* symbol = &symtab[i];
		if (symbol->value == address && (ST_CHECKTYPE(symbol, STT_NOTYPE) || ST_CHECKTYPE(symbol, STT_OBJECT) || ST_CHECKTYPE(symbol, STT_FUNC)))
			return strtab + symbol->name;
	}
	
	return SYMBOL_NOT_FOUND;
}

uint32_t kernelGetNearestSymbolByAddress(uint32_t address) {
	KernelSectionHeader_t* symtab_section = kernelLookupSectionByName(".symtab");
	if (symtab_section == NULL)
		return 0;

	uint32_t symtab_num = symtab_section->size / sizeof(KernelSymbol_t);
	KernelSymbol_t* symtab = (KernelSymbol_t*)symtab_section->addr;

	uint32_t nearest = 0;
	
	for (uint32_t i = 0; i < symtab_num; i++) {
		KernelSymbol_t* symbol = &symtab[i];
		if ((address - symbol->value) < (address - nearest))
			nearest = symbol->value;
	}
	
	return nearest;
}

typedef struct stackFrame {
	struct stackFrame* ebp;
	uint32_t eip;
} stackFrame_t;

void printStackFrame(uint32_t address) {
	if (!address)
		return;

    uint32_t nearestAddress = kernelGetNearestSymbolByAddress(address);
	
    if (nearestAddress) {
		uint8_t* name = kernelGetSymbolNameByAddress(nearestAddress);

		printf("	0x%08x -> %-25s at address 0x%08x\n", address, name, nearestAddress);
	} else {
		printf("	0x%08x\n", address);
	}
}

void stackTrace(uint32_t maxFrames) {
    printf("--[[					Stack trace begin					]]--\n");

	stackFrame_t* stk;
	__asm__ volatile("movl %%ebp, %0" : "=r"(stk));
	CPURegisters_t* interruptContext = getInterruptedContext();
	if (interruptContext) {
		printStackFrame(interruptContext->eip);
		stk = (stackFrame_t*)interruptContext->ebp;
		maxFrames--;
	}
    
    for(uint32_t frame = 0; frame < maxFrames; ++frame) {
		if (!stk) break;
		printStackFrame(stk->eip);
        stk = stk->ebp;
    }

    printf("--[[					Stack trace end		 				]]--\n");
}
