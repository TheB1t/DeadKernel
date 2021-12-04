#include "common.h"
#include "paging.h"

#define SYMBOL_NOT_FOUND ((uint8_t*)"UNKNOWN")

typedef struct {
	uint32_t	name;
	uint32_t	type;
	uint32_t	flags;
	uint32_t	addr;
	uint32_t	offset;
	uint32_t	size;
	uint32_t	link;
	uint32_t	info;
	uint32_t	addralign;
	uint32_t	entsize;
} KernelSectionHeader_t;

typedef struct {
	uint32_t	name;
	uint32_t	value;
	uint32_t	size;
	uint8_t		info;
	uint8_t		other;
	uint16_t	shndx;
} KernelSymbol_t;

#define ST_BIND(i)			((i) >> 4)
#define ST_TYPE(i)			((i) & 0xf)
#define ST_INFO(b,t)		(((b) << 4) + ((t) & 0xf))
#define ST_CHECKTYPE(d, t)	(ST_TYPE(((KernelSymbol_t*)d)->info) == (t))

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

extern KernelSectionHeader_t*	kernelSectionTable;
extern uint32_t					sectionTableSize;
extern uint32_t					sectionStringTableIndex;

KernelSectionHeader_t*	kernelSection(uint32_t idx);
uint8_t*				kernelLookupString(uint32_t offset);
KernelSectionHeader_t*	kernelLookupSectionByName(char* name);
uint8_t*				kernelGetSymbolNameByAddress(int32_t address);
uint32_t				kernelGetNearestSymbolByAddress(uint32_t address);
void					stackTrace(uint32_t maxFrames);
