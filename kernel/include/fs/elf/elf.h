#pragma once

#include <utils/common.h>
#include <io/screen.h>

typedef		uint32_t	ELF32_Addr_t;
typedef		uint32_t	ELF32_Off_t;
typedef 	uint16_t	ELF32_Half_t;
typedef		uint32_t	ELF32_Word_t;

#define ELF_MAGIC	(uint32_t)0x464C457F //0x7F + "ELF"

#define SYMBOL_NOT_FOUND ((uint8_t*)"UNKNOWN")

#define SHN_UNDEF	0

#define	ET_NONE		0		//No file type
#define	ET_REL		1		//Relocatable file
#define	ET_EXEC		2		//Executable file
#define	ET_DYN		3		//Shared object file
#define	ET_CORE		4		//Core file
#define	ET_LOOS		0xFE00	//Operating system-specific
#define	ET_HIOS		0xFEFF	//Operating system-specific
#define	ET_LOPROC	0xFF00	//Processor-specific
#define	ET_HIPROC	0xFFFF	//Processor-specific

#define	ELFCLASSNONE	0	//Invalid class
#define ELFCLASS32		1	//32-bit objects
#define ELFCLASS64		2	//64-bit objects

#define	ELFDATANONE 	0	//Invalid data encoding
#define	ELFDATA2LSB		1	//Least significant bit
#define	ELFDATA2MSB 	2	//Most significant bit

#define	ESHT_NULL		0
#define	ESHT_PROGBITS 	1
#define	ESHT_SYMTAB 	2
#define	ESHT_STRTAB		3
#define	ESHT_RELA		4
#define	ESHT_HASH		5
#define	ESHT_DYNAMIC 	6
#define	ESHT_NOTE	 	7
#define	ESHT_NOBITS 	8
#define	ESHT_REL 		9

#define	PT_NULL 	0
#define	PT_LOAD 	1
#define	PT_DYNAMIC 	2
#define	PT_INTERP 	3
#define	PT_NOTE 	4
#define	PT_SHLIB 	5
#define	PT_PHDR 	6
#define	PT_TLS 		7
#define	PT_LOOS 	0x60000000
#define	PT_HIOS 	0x6fffffff
#define	PT_LOPROC 	0x70000000
#define	PT_HIPROC 	0x7fffffff

typedef struct {
	struct {
		uint32_t magic;
		uint8_t class;
		uint8_t data;
		uint8_t version;
		uint8_t osabi;
		uint8_t abiversion;
		uint8_t unused[7];
	} ident;
	ELF32_Half_t	type;
	ELF32_Half_t	machine;
	ELF32_Word_t	version;
	ELF32_Addr_t	entry;
	ELF32_Off_t		phoff;
	ELF32_Off_t		shoff;
	ELF32_Word_t	flags;
	ELF32_Half_t	ehsize;
	ELF32_Half_t	phentsize;
	ELF32_Half_t	phnum;
	ELF32_Half_t	shentsize;
	ELF32_Half_t	shnum;
	ELF32_Half_t	shstrndx;	
} ELF32Header_t;

typedef struct {
	ELF32_Word_t	type;
	ELF32_Off_t		offset;
	ELF32_Addr_t	vaddr;
	ELF32_Addr_t	paddr;
	ELF32_Word_t	filesz;
	ELF32_Word_t	memsz;
	ELF32_Word_t	flags;
	ELF32_Word_t	align;
} ELF32ProgramHeader_t;

typedef struct {
	ELF32_Word_t	name;
	ELF32_Word_t	type;
	ELF32_Word_t	flags;
	ELF32_Addr_t	addr;
	ELF32_Off_t		offset;
	ELF32_Word_t	size;
	ELF32_Word_t	link;
	ELF32_Word_t	info;
	ELF32_Word_t	addralign;
	ELF32_Word_t	entsize;
} ELF32SectionHeader_t;

typedef struct {
	ELF32_Word_t	name;
	ELF32_Addr_t	value;
	ELF32_Word_t	size;
	uint8_t			info;
	uint8_t			other;
	ELF32_Half_t	shndx;
} ELF32Symbol_t;

#define ELF32_ST_BIND(i)	((i) >> 4)
#define ELF32_ST_TYPE(i)	((i) & 0xf)
#define ELF32_ST_INFO(b,t)	(((b) << 4) + ((t) & 0xf))

#define	STB_LOCAL	0
#define STB_GLOBAL 	1
#define STB_WEAK 	2
#define STB_LOOS 	10
#define STB_HIOS 	12
#define STB_LOPROC 	13
#define STB_HIPROC 	15

#define STT_NOTYPE 	0
#define STT_OBJECT 	1
#define STT_FUNC 	2
#define STT_SECTION 3
#define STT_FILE 	4
#define STT_COMMON 	5
#define STT_TLS 	6
#define STT_LOOS 	10
#define STT_HIOS 	12
#define STT_LOPROC 	13
#define STT_HIPROC 	15
   
uint8_t					ELF32CheckMagic(ELF32Header_t* hdr);
ELF32SectionHeader_t*	ELFSectionHeader(ELF32Header_t *hdr);
ELF32SectionHeader_t*	ELFSection(ELF32Header_t *hdr, uint32_t idx);
ELF32ProgramHeader_t*	ELFProgramHeader(ELF32Header_t *hdr);
ELF32ProgramHeader_t*	ELFProgram(ELF32Header_t *hdr, uint32_t idx);
uint8_t*				ELFStrTable(ELF32Header_t *hdr);
uint8_t*				ELFLookupString(ELF32Header_t *hdr, uint32_t offset);
ELF32SectionHeader_t*	ELFLookupSectionByName(ELF32Header_t *hdr, char* name);
uint8_t*				ELFGetSymbolNameByAddress(ELF32Header_t* hdr, uint32_t address);
uint32_t				ELFGetNearestSymbolByAddress(ELF32Header_t* hdr, uint32_t address);
