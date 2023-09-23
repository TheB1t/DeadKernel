#include <memory_managment/paging.h>
#include <memory_managment/kheap.h>
#include <utils/stackTrace.h>
#include <io/serial.h>
#include <multitasking/task.h>

#define INDEX_FROM_BIT(a)	(a / 32)
#define OFFSET_FROM_BIT(a)	(a % 32)
#define ADDR2FRAME(a)		(a / PAGE_SIZE)
#define FRAME2INDEX(a)		(a / 1024)
#define FRAME2OFFSET(a)		(a % 1024)
#define SET_FRAME(a)		(frames[INDEX_FROM_BIT(ADDR2FRAME(a))] |= (0x1 << OFFSET_FROM_BIT(ADDR2FRAME(a))))
#define CLEAR_FRAME(a)		(frames[INDEX_FROM_BIT(ADDR2FRAME(a))] &= ~(0x1 << OFFSET_FROM_BIT(ADDR2FRAME(a))))
#define READ_FRAME(a)		((frames[INDEX_FROM_BIT(ADDR2FRAME(a))] & (0x1 << OFFSET_FROM_BIT(ADDR2FRAME(a)))) > 0)


PageDir_t* kernelDir = 0;
PageDir_t* currentDir = 0;

uint32_t* frames;
uint32_t nframes;

extern uint32_t start;
extern uint32_t end;
//extern void copyPagePhysical(uint32_t, uint32_t);
extern uint32_t placementAddress;
extern Heap_t* kernelHeap;

extern void copyPagePhysical(uint32_t, uint32_t);

/*
 *	Searches for the first free physical memory frame and returns it
 */
static uint32_t firstFrame() {
	for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
		if (frames[i] != 0xFFFFFFFF)
			for (uint32_t j = 0; j < 32; j++) {
				uint32_t toTest = 0x1 << j;
				if (!(frames[i] & toTest))
					return (i * 32) + j;
			}
	}
}

/*
 *	Connects the page to the specified memory frame
 */
void linkFrame(Page_t* page, uint32_t alignedAddress, uint32_t isKernel, uint32_t isWriteable) {
	if (page->frame != 0)
		return;

	if (READ_FRAME(alignedAddress)) {
		WARN("Allocating busy frame!");
		// TODO: Need to rework this part
		// return;
	}

	SET_FRAME(alignedAddress);
	page->present	= 1;
	page->rw		= isWriteable	? 1 : 0;
	page->user		= isKernel		? 0 : 1;
	page->frame		= alignedAddress / PAGE_SIZE;	
	page->isKernel	= currentDir == kernelDir ? 1 : 0;
}

/*
 *	Connects the page to the first free frame
 */
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
	page->isKernel	= currentDir == kernelDir ? 1 : 0;

}

/*
 *	Frees the frame from the page
 */
void freeFrame(Page_t* page) {
	uint32_t frame = page->frame;
	if (!frame)
		return;

	CLEAR_FRAME(frame * PAGE_SIZE);
	page->present	= 0;
	page->frame		= 0x0;
}

/*
 *	Mirrors virtual memory to physical memory (VIRT_PAGE_ADDR == PHYS_FRAME_ADDR)
 */
uint8_t allocFramesMirrored(uint32_t start, uint32_t end, uint8_t makePage, uint32_t isKernel, uint32_t isWriteable) {
	start	^= start ? PAGE_SIZE - 1 : 0;
	end		^= PAGE_SIZE - 1;
	end		+= PAGE_SIZE;
	
	uint32_t len = end - start;
	// SERIAL_LOG_INFO(COM1, "Allocating [%08x:%08x] memory region (mirrored)", start, end);

	for (uint32_t i = start; i < start + len; i += PAGE_SIZE)		
		linkFrame(getPage(i, makePage, currentDir), i, isKernel, isWriteable);

	return 1;
}

/*
 *	Maps virtual memory to free physical frames
 */
void allocFrames(uint32_t start, uint32_t end, uint8_t makePage, uint32_t isKernel, uint32_t isWriteable) {
	uint32_t len = end - start;
	// SERIAL_LOG_INFO(COM1, "Allocating [%08x:%08x] memory region", start, end);
	for (uint32_t i = start; i < start + len; i += PAGE_SIZE)
		allocFrame(getPage(i, makePage, currentDir), isKernel, isWriteable);
}

/*
 *	Maps virtual memory to free physical frames (reversed)
 */
void allocFramesReversed(uint32_t start, uint32_t end, uint8_t makePage, uint32_t isKernel, uint32_t isWriteable) {
	uint32_t len = end - start;
	// LOG_INFO("Allocating [%08x:%08x] memory region", start, end);
	for (uint32_t i = start + len; i > start; i -= PAGE_SIZE)
		allocFrame(getPage(i, makePage, currentDir), isKernel, isWriteable);
}


/*
 *	Maps virtual memory to free physical frames
 */
void freeFrames(uint32_t start, uint32_t end) {
	uint32_t len = end - start;

	// SERIAL_LOG_INFO(COM1, "Freeing [%08x:%08x] memory region", start, end);
	for (uint32_t i = start; i < start + len; i += PAGE_SIZE)
		freeFrame(getPage(i, 0, currentDir));
}

/*
 *	Allocates memory for the kernel section
 */
void allocKernelSection(KernelSectionHeader_t* sec) {
	if (sec != NULL)
		allocFramesMirrored(sec->addr, sec->addr + sec->size, 1, 0, 0);
}

/*
 *	Allocates memory for the kernel section by name
 */
void allocKernelSectionByName(uint8_t* secName) {
	allocKernelSection(kernelLookupSectionByName(secName));
}

/*
 *	Initializes the paged memory mode,
 *	marks up the memory used by the kernel for its use
 */
void initPaging() {
	uint32_t memEndPageMB = 1024;
	uint32_t memEndPage = memEndPageMB * 1024 * 1024;

	nframes = ADDR2FRAME(memEndPage);
	frames	= (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	LOG_INFO("Page size: %d bytes", PAGE_SIZE);
	LOG_INFO("Page directory size: %d bytes", sizeof(PageDir_t));
	LOG_INFO("Free frames: %d", nframes);
	LOG_INFO("Memory available: %d MB (range %08x:%08x)", memEndPageMB, 0, memEndPage);
	
	kernelDir = (PageDir_t*)_kmalloc(sizeof(PageDir_t), 1, 0);
	currentDir = kernelDir;
	memset(kernelDir, 0, sizeof(PageDir_t));
	kernelDir->physicalAddr = (uint32_t)kernelDir->tablesPhysical;

	
	for (uint32_t i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE) 
		getPage(i, 1, kernelDir);

	uint32_t kheap_addr = kmalloc(sizeof(Heap_t));

	//If section table loaded
	if (isKernelSectionTableLoaded()) {
		//Allocating top memory
		allocFramesMirrored(0, (uint32_t)&start, 1, 0, 0);
		
		//Allocating kernel sections
		allocKernelSectionByName(".text");
		allocKernelSectionByName(".data");
		allocKernelSectionByName(".bss");
		allocKernelSectionByName(".symtab");
		allocKernelSectionByName(".strtab");
		allocKernelSectionByName(".shstrtab");
	} else {
		//Else, alloc all memory from zero to placementAddress
		allocFramesMirrored(0, placementAddress, 1, 0, 0);
	}
	//Page allocation for memory allocated before page mode was enabled
	allocFramesMirrored((uint32_t)&end, placementAddress + PAGE_SIZE, 1, 0, 0);

	//Allocating kernel heap
	allocFrames(KHEAP_START, KHEAP_START + KHEAP_MIN_SIZE, 1, 0, 0);

	registerInterruptHandler(14, pageFault);

	switchPageDir(kernelDir);
	
	kernelHeap = createHeap(kheap_addr, KHEAP_START, KHEAP_START + KHEAP_MIN_SIZE, 0xCFFFF000, 0, 0);
}

/*
 *	Switches the page directory
 */
PageDir_t* switchPageDir(PageDir_t* dir) {
	PageDir_t* oldDir = currentDir;
	currentDir = dir;
	asm volatile ("mov %0, %%cr3" : : "r"(dir->physicalAddr));
	uint32_t cr0;
	asm volatile ("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
	return oldDir;
}

/*
 *	Retrieves a page from the pages directory
 */
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

/*
 *	Releases all frames occupied by the page table
 */
void freeTable(PageTable_t* table) {
    for (uint32_t i = 0; i < PAGES_IN_TABLE; i++) {
        if (table->pages[i].frame) {
            freeFrame(&table->pages[i]);
        }
    }

    kfree(table);
}

/*
 *	Page fault interrupt handler
 */
void pageFault(CPURegisters_t* regs) {
    uint32_t faultingAddress;
    asm volatile ("mov %%cr2, %0" : "=r" (faultingAddress));

    char* err = "Unknown error";
	bool resolved = false;
    
    if (!(regs->err_code & 0x1)) {
    	err = "Page not present";

		PageDir_t* savedDir = NULL;

		if (isTaskingInit()) {
			if (regs->cr3 == currentTask->pageDir->physicalAddr)
				if (currentDir != currentTask->pageDir)
					savedDir = switchPageDir(currentTask->pageDir);

		} else if (regs->cr3 != currentDir->physicalAddr) {
			savedDir = switchPageDir(currentDir);
		}
		
		if (currentDir == kernelDir) {
			allocFramesMirrored(faultingAddress, faultingAddress + PAGE_SIZE, 1, 0, 0);
		} else {
			allocFrames(faultingAddress, faultingAddress + PAGE_SIZE, 1, 0, 1);
		}

		if (savedDir) {
			switchPageDir(savedDir);
		}

		resolved = true;
		
	} else if (regs->err_code & 0x2)
    	err = "Page is read-only";
    	
    else if (regs->err_code & 0x4)
    	err = "Processor in user-mode";
    	 
    else if (regs->err_code & 0x8)
    	err = "Overwrite CPU-reserved bits";

	if (!resolved) {
    	LOG_INFO("Page fault [0x%x] %s (eip 0x%08x) (unresolved)", faultingAddress, err, regs->eip);
		PANIC("Page fault");	
	}
}

/*
 *	Clones the page table
 */
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
		table->pages[i].isKernel	= src->pages[i].isKernel	? 1 : 0;
		
		copyPagePhysical(src->pages[i].frame * PAGE_SIZE, table->pages[i].frame * PAGE_SIZE);
	}

	return table;
}

/*
 *	Clones the page directory
 */
PageDir_t* cloneDir(PageDir_t* src) {
	uint32_t phys;

	PageDir_t* dir = (PageDir_t*)_kmalloc(sizeof(PageDir_t), 1, &phys);
	memset(dir, 0, sizeof(PageDir_t));

	uint32_t offset = (uint32_t)dir->tablesPhysical - (uint32_t)dir; //Calculating offset in structure
	dir->physicalAddr = phys + offset; //Physical address of dir + offset
	
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

/*
 *  Releases all resources occupied by the page directory
 */
void freeDir(PageDir_t* dir) {
    for (uint32_t i = 0; i < TABLES_IN_DIR; i++) {
        if (dir->tables[i])
            if (dir->tables[i] != kernelDir->tables[i])
                freeTable(dir->tables[i]);
    }

    kfree(dir);
}
