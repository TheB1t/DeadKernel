#pragma once

#include <utils/common.h>
#include <interrupts/isr.h>

#define PAGE_SIZE		(4096)
#define	PAGES_IN_TABLE	(1024)
#define TABLES_IN_DIR	(1024)

typedef struct {
	uint32_t present	:1;
	uint32_t rw			:1;
	uint32_t user		:1;
	uint32_t accessed	:1;
	uint32_t dirty		:1;
	uint32_t unused		:6;
	uint32_t isKernel	:1;
	uint32_t frame		:20;
} Page_t;

typedef struct {
	Page_t pages[PAGES_IN_TABLE];
} PageTable_t;

typedef struct {
	PageTable_t* 	tables[TABLES_IN_DIR];
	uint32_t		tablesPhysical[TABLES_IN_DIR];
	uint32_t		physicalAddr;
} PageDir_t;

void allocFrame(Page_t* page, uint32_t isKernel, uint32_t isWriteable);
void freeFrame(Page_t* page);

uint8_t allocFramesMirrored(uint32_t start, uint32_t end, uint8_t makePage, uint32_t isKernel, uint32_t isWriteable);
void allocFrames(uint32_t start, uint32_t end, uint8_t makePage, uint32_t isKernel, uint32_t isWriteable);
void allocFramesReversed(uint32_t start, uint32_t end, uint8_t makePage, uint32_t isKernel, uint32_t isWriteable);

void initPaging();
PageDir_t* switchPageDir(PageDir_t* dir);
Page_t* getPage(uint32_t address, uint8_t make, PageDir_t* dir);
void freeTable(PageTable_t* table);
void pageFault(CPURegisters_t* regs);
PageDir_t* cloneDir(PageDir_t* src);
void freeDir(PageDir_t* dir);