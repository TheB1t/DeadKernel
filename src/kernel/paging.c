#include "paging.h"
#include "kheap.h"

#define INDEX_FROM_BIT(a)	(a / 32)
#define OFFSET_FROM_BIT(a)	(a % 32)
#define ADDR2FRAME(a)		(a / PAGE_SIZE)
#define FRAME2INDEX(a)		(a / 1024)
#define FRAME2OFFSET(a)		(a % 1024)
#define SET_FRAME(a)		(frames[INDEX_FROM_BIT(a)] |= (0x1 << OFFSET_FROM_BIT(a)))
#define CLEAR_FRAME(a)		(frames[INDEX_FROM_BIT(a)] &= ~(0x1 << OFFSET_FROM_BIT(a)))
#define READ_FRAME(a)		(frames[INDEX_FROM_BIT(a)] & (0x1 << OFFSET_FROM_BIT(a)))


PageDir_t* kernelDir = 0;
PageDir_t* currentDir = 0;

uint32_t* frames;
uint32_t nframes;

extern uint32_t placementAddress;
extern Heap_t* kernelHeap;

static uint32_t firstFrame() {
	for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
		if (frames[i] != 0xFFFFFFFF)
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

	SET_FRAME(frame);
	page->present	= 1;
	page->rw		= (isWriteable) ? 1 : 0;
	page->user		= (isKernel) ? 0 : 1;
	page->frame		= frame;
}

void freeFrame(Page_t* page) {
	uint32_t frame = page->frame;
	if (!(frame))
		return;

	CLEAR_FRAME(frame);
//	page->present	= 0;
	page->frame		= 0x0;
}

void initPaging() {
	uint32_t memEndPage = 16 * 1024 * 1024; //16 MB

	nframes = ADDR2FRAME(memEndPage);
	frames	= (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	kernelDir = (PageDir_t*)_kmalloc(sizeof(PageDir_t), 1, 0);

	for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE)
		getPage(i, 1, kernelDir);

	uint32_t kheap_addr = kmalloc(sizeof(Heap_t));
	
	for (uint32_t i = 0; i < placementAddress; i += PAGE_SIZE)
		allocFrame(getPage(i, 1, kernelDir), 0, 0);

	for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE)
		allocFrame(getPage(i, 0, kernelDir), 0, 0);
			
	registerInterruptHandler(14, pageFault);

	switchPageDir(kernelDir);

	kernelHeap = createHeap(kheap_addr, KHEAP_START, KHEAP_START + KHEAP_MIN_SIZE, 0xCFFFF000, 0, 0);
}

void switchPageDir(PageDir_t* dir) {
	uint32_t cr0;
	currentDir = dir;
	asm volatile ("mov %0, %%cr3" : : "r"(&dir->tablesPhysical));
	asm volatile ("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

Page_t* getPage(uint32_t address, uint8_t make, PageDir_t* dir) {
	uint32_t frame			= ADDR2FRAME(address);
	uint32_t page_index		= FRAME2INDEX(frame);
	uint32_t page_offset	= FRAME2OFFSET(frame);
	
	PageTable_t** table 	= &dir->tables[page_index];
	uint32_t* tablePhys		= &dir->tablesPhysical[page_index];
	
	if (*table)
		return &(*table)->pages[page_offset];
	else if (make) {
		uint32_t physAddress;
		*table = (PageTable_t*)_kmalloc(sizeof(PageTable_t), 1, &physAddress);
		*tablePhys = physAddress | 0x7;
		return &(*table)->pages[page_offset];
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

