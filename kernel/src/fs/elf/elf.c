#include <fs/elf/elf.h>

inline bool ELF32CheckMagic(ELF32Header_t* hdr) {
	return (hdr->ident.magic & ELF_MAGIC) == ELF_MAGIC;
}

ELF32SectionHeader_t* ELFLookupSectionByName(ELF32Obj_t* hdr, char* name) {
	for (uint32_t i = 0; i < ELF32_SECTAB_NENTRIES(hdr); i++) {
		ELF32SectionHeader_t* section = ELF32_ENTRY(ELF32_TABLE(hdr, sec), i);

		uint8_t* section_name = (uint8_t*)ELF32_TABLE(hdr, hstr) + section->name;
		if (strcmp(section_name, name))
			continue;

		return section; 
	}
	
	return NULL;
}

ELF32SectionHeader_t* ELFLookupSectionByType(ELF32Obj_t* hdr, uint8_t type) {
	for (uint32_t i = 0; i < ELF32_SECTAB_NENTRIES(hdr); i++) {
		ELF32SectionHeader_t* section = ELF32_ENTRY(ELF32_TABLE(hdr, sec), i);

		if (section->type != type)
			continue;

		return section; 
	}
	
	return NULL;
}

ELF32Symbol_t* ELFLookupSymbolByName(ELF32Obj_t* hdr, uint8_t type, char* name) {
	if (ELF32_SECTAB_NENTRIES(hdr) == 0) goto _error;
	if (ELF32_SYMTAB_NENTRIES(hdr) == 0) goto _error;
	if (ELF32_STRTAB_SIZE(hdr) == 0) goto _error;

	for (uint32_t i = 0; i < ELF32_SYMTAB_NENTRIES(hdr); i++) {
		ELF32Symbol_t* symbol = ELF32_ENTRY(ELF32_TABLE(hdr, sym), i);

		if (!STT_CHECKTYPE(symbol, type))
			continue;

		uint8_t* symbol_name = (uint8_t*)ELF32_TABLE(hdr, str) + symbol->name;
		if (strcmp(symbol_name, name) == 0)
			return symbol;
	}
	
_error:
	return NULL;
}

ELF32Symbol_t* ELFGetNearestSymbolByAddress(ELF32Obj_t* hdr, uint8_t type, uint32_t address) {
	if (ELF32_SECTAB_NENTRIES(hdr) == 0) goto _error;
	if (ELF32_SYMTAB_NENTRIES(hdr) == 0) goto _error;
	if (address < hdr->start || address > hdr->end) goto _error;

	ELF32Symbol_t* nearest = NULL;
	
	for (uint32_t i = 0; i < ELF32_SYMTAB_NENTRIES(hdr); i++) {
		ELF32Symbol_t* symbol = ELF32_ENTRY(ELF32_TABLE(hdr, sym), i);

		if (!STT_CHECKTYPE(symbol, type))
			continue;

		if (!nearest)
			nearest = symbol;
			
		if ((address - symbol->value) < (address - nearest->value))
			nearest = symbol;
	}
	
	return nearest;
_error:
	return NULL;
}

bool ELFLoad(uint32_t address, ELF32Obj_t* out) {
	ELF32Header_t* header = (ELF32Header_t*)address;
	if (!ELF32CheckMagic(header))
		return false;

	out->header = header;

	ELF32_TABLE(out, sec)			= ELF32_SECTAB(out);
	ELF32_TABLE_SIZE(out, sec)		= ELF32_HEADER(out)->shnum;

	ELF32_TABLE(out, prog)			= ELF32_PROGTAB(out);
	ELF32_TABLE_SIZE(out, prog)		= ELF32_HEADER(out)->phnum;

	ELF32_TABLE(out, hstr)			= ELF32_HSTRTAB(out);

	ELF32SectionHeader_t* symtab	= ELFLookupSectionByType(out, ESHT_SYMTAB);
	ELF32_TABLE(out, sym)			= symtab ? (ELF32Symbol_t*)((uint8_t*)out->header + symtab->offset) : NULL;
	ELF32_TABLE_SIZE(out, sym)		= symtab->size / sizeof(ELF32Symbol_t);

	ELF32SectionHeader_t* strtab	= ELFLookupSectionByType(out, ESHT_STRTAB);
	ELF32_TABLE(out, str)			= strtab ? (uint8_t*)out->header + strtab->offset : NULL;
	ELF32_TABLE_SIZE(out, str)		= strtab ? strtab->size : 0;

	ELF32Symbol_t* startsym			= ELFLookupSymbolByName(out, STT_NOTYPE, "__code");
	out->start						= startsym ? startsym->value : 0;

	ELF32Symbol_t* endsym			= ELFLookupSymbolByName(out, STT_NOTYPE, "__end");
	out->end						= endsym ? endsym->value : 0;

	return true;
}