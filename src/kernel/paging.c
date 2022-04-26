#include "paging.h"
#include "kheap.h"

#define INDEX_FROM_BIT(a)	(a / 32)
#define OFFSET_FROM_BIT(a)	(a % 32)
#define ADDR2FRAME(a)		(a / PAGE_SIZE)
#define FRAME2INDEX(a)		(a / 1024)
#define FRAME2OFFSET(a)		(a % 1024)
#define SET_FRAME(a)		(frames[INDEX_FROM_BIT(ADDR2FRAME(a))] |= (0x1 << OFFSET_FROM_BIT(ADDR2FRAME(a))))
#define CLEAR_FRAME(a)		(frames[INDEX_FROM_BIT(ADDR2FRAME(a))] &= ~(0x1 << OFFSET_FROM_BIT(ADDR2FRAME(a))))
#define READ_FRAME(a)		(frames[INDEX_FROM_BIT(ADDR2FRAME(a))] & (0x1 << OFFSET_FROM_BIT(ADDR2FRAME(a))))


PageDir_t*	kernelDir = 0;
PageDir_t*	currentDir = 0;

uint32_t*	frames;
uint32_t	nframes;

extern uint32_t	placementAddress;
extern Heap_t*	kernelHeap;
extern void		copyPagePhysical(uint32_t, uint32_t);

static uint32_t firstFrame() {
	for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
		if (frames[i] == 0xFFFFFFFF)
			continue;

		for (uint32_t j = 0; j < 32; j++) {
			if (!(frames[i] & (0x1 << j))) 
				return (i * 32) + j;
		}
	}
}

void allocFrame(Page_t* page, uint32_t isKernel, uint32_t isWriteable) {
	if (page->frame != 0)
		return;

	uint32_t frame = firstFrame();
	if (frame == (uint32_t)-1)
		PANIC("No free memory frames");

	SET_FRAME(frame * PAGE_SIZE);
	page->present	= 1;
	page->rw		= isWriteable	? 1 : 0;
	page->user		= isKernel		? 0 : 1;
	page->frame		= frame;
}

void freeFrame(Page_t* page) {
	uint32_t frame = page->frame;
	if (!frame)
		return;

	CLEAR_FRAME(frame);
	page->present	= 0;
	page->frame		= 0x0;
}

void initPaging() {
	uint32_t memEndPage = 128 * 1024 * 1024; //16 MB

	nframes = ADDR2FRAME(memEndPage);
	frames	= (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	kernelDir = (PageDir_t*)_kmalloc(sizeof(PageDir_t), 1, 0);
	memset(kernelDir, 0, sizeof(PageDir_t));
	kernelDir->physicalAddr = (uint32_t)kernelDir->tablesPhysical;
	
	for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE)
		getPage(i, 1, kernelDir);

	uint32_t kheap_addr = kmalloc(sizeof(Heap_t));
	
	for (uint32_t i = 0; i < placementAddress + PAGE_SIZE; i += PAGE_SIZE)
		allocFrame(getPage(i, 1, kernelDir), 0, 0);

	for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE)
		allocFrame(getPage(i, 1, kernelDir), 0, 0);
			
	registerInterruptHandler(14, pageFault);

	switchPageDir(kernelDir);

	kernelHeap = createHeap(kheap_addr, KHEAP_START, KHEAP_START + KHEAP_MIN_SIZE, 0xCFFFF000, 0, 0);
}

void switchPageDir(PageDir_t* dir) {
	currentDir = dir;
	asm volatile ("mov %0, %%cr3" : : "r"(dir->physicalAddr));
	uint32_t cr0;
	asm volatile ("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

Page_t* getPage(uint32_t address, uint8_t make, PageDir_t* dir) {
	uint32_t frame			= ADDR2FRAME(address);
	uint32_t page_index		= FRAME2INDEX(frame);
	uint32_t page_offset	= FRAME2OFFSET(frame);
	
	if (dir->tables[page_index])
		return &dir->tables[page_index]->pages[page_offset];
	else if (make) {
		uint32_t physAddress;
		dir->tables[page_index]			= (PageTable_t*)_kmalloc(sizeof(PageTable_t), 1, &physAddress);
		memset(dir->tables[page_index], 0, PAGE_SIZE);
		dir->tablesPhysical[page_index]	= physAddress | 0x7;
		return &dir->tables[page_index]->pages[page_offset];
	}

	return 0;
}

void pageFault(registers_t regs) {
    uint32_t faultingAddress;
    asm volatile ("mov %%cr2, %0" : "=r" (faultingAddress));

    char* err = "Unknown error";
    
    if (!(regs.err_code & 0x1)) 
    	err = "Page not present";
    	
    if (regs.err_code & 0x2)
    	err = "Page is read-only";
    	
    if (regs.err_code & 0x4)
    	err = "Processor in user-mode";
    	 
    if (regs.err_code & 0x8)
    	err = "Overwrite CPU-reserved bits";

	printf("Page fault [0x%x] %s\n", faultingAddress, err);
    PANIC("Page fault");	
}

static PageTable_t* cloneTable(PageTable_t* src, uint32_t* physAddr) {
	PageTable_t* table = (PageTable_t*)_kmalloc(sizeof(PageTable_t), 1, physAddr);

	memset(table, 0, sizeof(PageTable_t));

	for (uint32_t i = 0; i < PAGES_IN_TABLE; i++) {
		if (!src->pages[i].frame) 
			continue;
			
		allocFrame(&table->pages[i], 0, 0);

		table->pages[i].present		= src->pages[i].present		? 1 : 0;
		table->pages[i].rw			= src->pages[i].rw			? 1 : 0;
		table->pages[i].user		= src->pages[i].user		? 1 : 0;
		table->pages[i].accessed	= src->pages[i].accessed	? 1 : 0;
		table->pages[i].dirty		= src->pages[i].dirty		? 1 : 0;

		copyPagePhysical(src->pages[i].frame * PAGE_SIZE, table->pages[i].frame * PAGE_SIZE);
	}

	return table;
}

PageDir_t* cloneDir(PageDir_t* src) {
	uint32_t phys;

	PageDir_t* dir = (PageDir_t*)_kmalloc(sizeof(PageDir_t), 1, &phys);
	memset(dir, 0, sizeof(PageDir_t));

	uint32_t offset = (uint32_t)dir->tablesPhysical - (uint32_t)dir;
	dir->physicalAddr = phys + offset;
	
	for (uint32_t i = 0; i < TABLES_IN_DIR; i++) {
		if (!src->tables[i])
			continue;

		if (kernelDir->tables[i] == src->tables[i]) {
			dir->tables[i]			= src->tables[i];
			dir->tablesPhysical[i]	= src->tablesPhysical[i];
		} else {
			phys = 0;
			dir->tables[i]			= cloneTable(src->tables[i], &phys);
			dir->tablesPhysical[i]	= phys | 0x07;
		}
	}
	return dir;
}
