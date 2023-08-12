#include <fs/elf/elf.h>

inline uint8_t ELF32CheckMagic(ELF32Header_t* hdr) {
	return (hdr->ident.magic & ELF_MAGIC) == ELF_MAGIC;
}

inline ELF32SectionHeader_t* ELFSectionHeader(ELF32Header_t *hdr) {
	return (ELF32SectionHeader_t*)((uint8_t*)hdr + hdr->shoff);
}
 
inline ELF32SectionHeader_t* ELFSection(ELF32Header_t *hdr, uint32_t idx) {
	return &ELFSectionHeader(hdr)[idx];
}

inline ELF32ProgramHeader_t* ELFProgramHeader(ELF32Header_t *hdr) {
	return (ELF32ProgramHeader_t*)((uint8_t*)hdr + hdr->phoff);
}
 
inline ELF32ProgramHeader_t* ELFProgram(ELF32Header_t *hdr, uint32_t idx) {
	return &ELFProgramHeader(hdr)[idx];
}

uint8_t* ELFStrTable(ELF32Header_t *hdr) {
	if (hdr->shstrndx == SHN_UNDEF)
		return NULL;
	return (uint8_t*)hdr + ELFSection(hdr, hdr->shstrndx)->offset;
}
 
uint8_t* ELFLookupString(ELF32Header_t *hdr, uint32_t offset) {
	uint8_t* strtab = ELFStrTable(hdr);
	if (strtab == NULL)
		return NULL;
	return (strtab + offset);
}

ELF32SectionHeader_t* ELFLookupSectionByName(ELF32Header_t *hdr, char* name) {
	for (uint32_t i = 0; i < hdr->shnum; i++) {
		ELF32SectionHeader_t* section = ELFSection(hdr, i);
		if (strcmp(ELFLookupString(hdr, section->name), name))
			continue;

		return section; 
	}
	
	return NULL;
}

uint8_t* ELFGetSymbolNameByAddress(ELF32Header_t* hdr, uint32_t address) {
	if (address == 0)
		return SYMBOL_NOT_FOUND;
		
	ELF32SectionHeader_t* strtab_section = ELFLookupSectionByName(hdr, ".strtab");
	if (strtab_section == NULL)
		return SYMBOL_NOT_FOUND;

	uint8_t* strtab = (uint8_t*)hdr + strtab_section->offset;
	
	ELF32SectionHeader_t* symtab_section = ELFLookupSectionByName(hdr, ".symtab");
	if (symtab_section == NULL)
		return SYMBOL_NOT_FOUND;
		
	uint32_t symtab_num = symtab_section->size / sizeof(ELF32Symbol_t);
	ELF32Symbol_t* symtab = (ELF32Symbol_t*)((uint8_t*)hdr + symtab_section->offset);
	
	for (uint32_t i = 0; i < symtab_num; i++) {
		ELF32Symbol_t* symbol = &symtab[i];
		if (symbol->value == address)
			return strtab + symbol->name;
	}
	
	return SYMBOL_NOT_FOUND;
}

uint32_t ELFGetNearestSymbolByAddress(ELF32Header_t* hdr, uint32_t address) {
	ELF32SectionHeader_t* symtab_section = ELFLookupSectionByName(hdr, ".symtab");
	if (symtab_section == NULL)
		return 0;
		
	uint32_t symtab_num = symtab_section->size / sizeof(ELF32Symbol_t);
	ELF32Symbol_t* symtab = (ELF32Symbol_t*)((uint8_t*)hdr + symtab_section->offset);

	uint32_t nearest = 0;
	
	for (uint32_t i = 0; i < symtab_num; i++) {
		ELF32Symbol_t* symbol = &symtab[i];
		if ((address - symbol->value) < (address - nearest))
			nearest = symbol->value;
	}
	
	return nearest;
}

