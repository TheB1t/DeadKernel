#pragma once

#include "common.h"
#include "isr.h"

#define PAGE_SIZE		(4096)
#define	PAGES_IN_TABLE	(1024)
#define TABLES_IN_DIR	(1024)

typedef struct {
	uint32_t	present	:1;
	uint32_t	rw			:1;
	uint32_t	user		:1;
	uint32_t	accessed	:1;
	uint32_t	dirty		:1;
	uint32_t	unused		:7;
	uint32_t	frame		:20;
} Page_t;

typedef struct {
	Page_t		pages[TABLES_IN_DIR];
} PageTable_t;

typedef struct {
	PageTable_t* 	tables[PAGES_IN_TABLE];
	uint32_t		tablesPhysical[PAGES_IN_TABLE];
	uint32_t		physicalAddr;
} PageDir_t;

void		allocFrame(Page_t* page, uint32_t isKernel, uint32_t isWriteable);
void		freeFrame(Page_t* page);

void		initPaging();
void		switchPageDir(PageDir_t* dir);
Page_t*		getPage(uint32_t address, uint8_t make, PageDir_t* dir);
void		pageFault(registers_t regs);
PageDir_t*	cloneDir(PageDir_t* src);
