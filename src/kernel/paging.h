#pragma once

#include "common.h"
#include "isr.h"

#define PAGE_SIZE (0x1000)

typedef struct {
	uint32_t present	:1;
	uint32_t rw			:1;
	uint32_t user		:1;
	uint32_t accessed	:1;
	uint32_t dirty		:1;
	uint32_t unused		:7;
	uint32_t frame		:20;
} Page_t;

typedef struct {
	Page_t pages[1024];
} PageTable_t;

typedef struct {
	PageTable_t* tables[1024];
	uint32_t		tablesPhysical[1024];
	uint32_t		physicalAddr;
} PageDir_t;

void allocFrame(Page_t* page, uint32_t isKernel, uint32_t isWriteable);
void freeFrame(Page_t* page);

void initPaging();
void switchPageDir(PageDir_t* dir);
Page_t* getPage(uint32_t address, uint8_t make, PageDir_t* dir);
void pageFault(registers_t regs);
