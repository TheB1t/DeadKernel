#include <multitasking/task.h>

typedef struct stackFrame {
	struct stackFrame* ebp;
	uint32_t eip;
} stackFrame_t;

Task_t* kernelTask		= 0;
Task_t* currentTask		= 0;

TaskQueue_t mainQueue;

extern void loadEntry();

uint32_t nextPID = 0;

void		insertTask(TaskQueue_t* queue, Task_t* task);
void		cutTask(Task_t* task);
void		initQueue(TaskQueue_t* queue);
uint32_t	getStackBegin();
void		createStack(uint32_t newStart, uint32_t size, bool isKernel);
void		cloneStack(uint32_t oldStart, uint32_t newStart, uint32_t size, bool isKernel, uint32_t* ESP, uint32_t* EBP);
void		moveStack(uint32_t oldStart, uint32_t newStart, uint32_t size);
void		yieldInterrupt(CPURegisters_t* regs);

void initTasking() {
	DISABLE_INTERRUPTS();
	
	// globalMutex = mutex_alloc();
	initQueue(&mainQueue);

	moveStack(getStackBegin(), 0xE0000000, 0x8000);

	kernelTask					= allocTask();
	
	kernelTask->elf_obj			= KERNEL_TABLE_OBJ;
	kernelTask->pageDir			= kernelDir;
	kernelTask->status			= TS_CREATED;
	kernelTask->time			= 0;

	kernelTask->regs.cs			= GDT_DESC_SEG(GDT_DESC_KERNEL_CODE, PL_RING0);
	kernelTask->regs.ds			= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0);
	kernelTask->regs.val1		= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0);
	kernelTask->regs.cr3		= kernelDir->physicalAddr;
	kernelTask->heap			= kernelHeap;

	asm volatile("				\
				  mov %%cs, %0;	\
				  mov %%ds, %1;	\
				  pushf;		\
				  pop %2;		\
				":"=r" (kernelTask->regs.cs),
				  "=r" (kernelTask->regs.ds),
				  "=r" (kernelTask->regs.eflags));

	runTask(kernelTask);

	registerInterruptHandler(64, &yieldInterrupt);
	ENABLE_INTERRUPTS();
}

static void copyContext(CPURegisters_t* dst, CPURegisters_t* src) {
	#define COPY(dst, src, field)	(dst)->field = (src)->field

	COPY(dst, src, _esp);

	COPY(dst, src, eax);
	COPY(dst, src, ebx);
	COPY(dst, src, ecx);
	COPY(dst, src, edx);
	COPY(dst, src, esi);
	COPY(dst, src, edi);	

	COPY(dst, src, ebp);
	COPY(dst, src, cr3);
	COPY(dst, src, ds);

	COPY(dst, src, eip);
	COPY(dst, src, cs);
	COPY(dst, src, eflags);

	COPY(dst, src, val0);
	COPY(dst, src, val1);

	dst->int_no		= 0;
	dst->err_code	= 0;
}

void switchTask(CPURegisters_t* regs) {
	if (!isTaskingInit())
			return;

	Task_t* oldTask = currentTask;
	copyContext(&currentTask->regs, regs);

	while (1) {
		switch (currentTask->status) {
			case TS_YIELD:
			case TS_RUNNING:
				if (currentTask->regs.ecx == 0xFE11DEAD) {
					currentTask->regs.ecx = 0;
					currentTask->status = TS_ZOMBIE;
					currentTask->exitcode = currentTask->regs.eax;
					LOG_INFO("PID %d finished with exitcode 0x%08x in Ring %d", currentTask->id, currentTask->exitcode, currentTask->regs.cs & 3);
				} else {
					currentTask->status = TS_IDLE;
				}
				break;

			case TS_IDLE:
				currentTask->status = TS_RUNNING;
				break;

			case TS_ZOMBIE:
			case TS_FINISHED:
				if (currentTask == kernelTask) {
					if (!currentTask->next) {
						LOG_INFO("Last task finished. Kernel be halted soon ;)");
						//Set self-destruct settings
						currentTask->regs.ebx = 0x50000000;
						currentTask->status = TS_RUNNING;
					}
				} else {
					destroyTask(currentTask);
				}
				break;			
		}

		if (currentTask->status == TS_RUNNING)
			break;
		
		if (currentTask->next) {
			currentTask = currentTask->next;
			break;
		}

		currentTask = QUEUE_FIRST_TASK(mainQueue);
	}

	setKernelStack(kernelTask->regs._esp);

	copyContext(regs, &currentTask->regs);
}

void copyFromUser(void* ptr0, void* userPtr, uint32_t size) {
	DISABLE_INTERRUPTS();

	PageDir_t* savedDir = switchPageDir(currentTask->pageDir);

	memcpy(ptr0, userPtr, size);

	switchPageDir(savedDir);

	ENABLE_INTERRUPTS();
}

void copyToUser(void* ptr0, void* userPtr, uint32_t size) {
	DISABLE_INTERRUPTS();

	PageDir_t* savedDir = switchPageDir(currentTask->pageDir);

	memcpy(userPtr, ptr0, size);

	switchPageDir(savedDir);

	ENABLE_INTERRUPTS();
}

Task_t* makeTaskFromELF(ELF32Obj_t* hdr) {
	DISABLE_INTERRUPTS();

	Task_t* newTask			= allocTask();

	newTask->elf_obj		= hdr;
	newTask->status			= TS_CREATED;
	newTask->time			= 0;
	newTask->regs.eip		= hdr->header->entry;

	PageDir_t* clonedDir	= cloneDir(kernelTask->pageDir);
	
	newTask->pageDir	= clonedDir;
	newTask->regs.cr3	= clonedDir->physicalAddr;
	newTask->regs.val0	= BASE_PROCESS_ESP;
	newTask->regs.cs	= GDT_DESC_SEG(GDT_DESC_USER_CODE, PL_RING3);
	newTask->regs.ds	= GDT_DESC_SEG(GDT_DESC_USER_DATA, PL_RING3);
	newTask->regs.val1  = GDT_DESC_SEG(GDT_DESC_USER_DATA, PL_RING3);
	
	newTask->regs.eflags.r0 = 1;
	newTask->regs.eflags.r1 = 0;
	newTask->regs.eflags.r2 = 0;
	newTask->regs.eflags.IF = 1;

	uint32_t heap_addr = kmalloc(sizeof(Heap_t));

	PageDir_t* savedDir = switchPageDir(clonedDir);

	createStack(BASE_PROCESS_ESP, PROCESS_STACK_SIZE, false);
	
	allocFrames(HEAP_START, HEAP_START + HEAP_MIN_SIZE, 1, 0, 1);
	newTask->heap = createHeap(heap_addr, clonedDir, HEAP_START, HEAP_START + HEAP_MIN_SIZE, 0xA0000000, 0, 0);

	for (uint32_t i = 0; i < ELF32_SECTAB_NENTRIES(hdr); i++) {
		ELF32SectionHeader_t* section = ELF32_ENTRY(ELF32_TABLE(hdr, sec), i);

		if (section->type == ESHT_PROGBITS || section->type == ESHT_NOBITS) {
			uint8_t* section_name = (uint8_t*)ELF32_TABLE(hdr, hstr) + section->name;

			uint32_t start = section->addr & -(PAGE_SIZE - 1);
			uint32_t end = (section->addr + section->size) & -(PAGE_SIZE - 1);

			uint32_t appendix = section->addr & (PAGE_SIZE - 1);

			if (appendix)
				start -= PAGE_SIZE;

			// serialprintf(COM1, "Allocating %s (%08x#%08x:%08x) %s\n", section_name, appendix, start, end, (section->flags & SHF_WRITE) > 0 ? "RW" : "RO");
			allocFrames(start, end, 1, 0, (section->flags & SHF_WRITE) > 0);
		}
	}

	for (uint32_t i = 0; i < ELF32_PROGTAB_NENTRIES(hdr); i++) {

		ELF32ProgramHeader_t* ph = ELF32_PROGRAM(hdr, i);
		if (ph->type == PT_LOAD)
			memcpy((uint8_t*)ph->vaddr, ((uint8_t*)hdr->header) + ph->offset, ph->filesz);
	}

	switchPageDir(savedDir);
	
	ENABLE_INTERRUPTS();
	return newTask;
}

int32_t runTask(Task_t* task) {
	if (task->status != TS_CREATED || task->status == TS_STOPPED)
		return -1;

	DISABLE_INTERRUPTS();
	
	task->id = task->id ? task->id : nextPID++;

	insertTask(&mainQueue, task);

	task->status = TS_IDLE;

	if (!currentTask)
		currentTask = task;

	ENABLE_INTERRUPTS();
	return task->id;	
}

void stopTask(Task_t* task) {
	if (task->status == TS_CREATED)
		return;

	DISABLE_INTERRUPTS();
	
	cutTask(task);
	
	if (task->status != TS_FINISHED)
		task->status == TS_STOPPED;
	
	ENABLE_INTERRUPTS();
}

void destroyTask(Task_t* task) {
	DISABLE_INTERRUPTS();

	stopTask(task);

	freeDir(task->pageDir);

	freeTask(task);

	ENABLE_INTERRUPTS();
}

void insertTask(TaskQueue_t* queue, Task_t* task) {
	Task_t* q = (Task_t*)queue;
	while (q->next)
		q = q->next;

	q->next = task;
	task->prev = q;
}

void cutTask(Task_t* task) {
	if (task->prev)
		task->prev->next = task->next;
					
	if (task->next)
		task->next->prev = task->prev;

	task->prev = 0;
	task->next = 0;

}

void initQueue(TaskQueue_t* queue) {
	memset(queue, 0, sizeof(TaskQueue_t));
}

Task_t* allocTask() {
	Task_t* task = (Task_t*)kmalloc(sizeof(Task_t));
	memset((uint8_t*)task, 0, sizeof(Task_t));
	return task;
}

void freeTask(Task_t* task) {
	kfree(task);
}

uint32_t getStackBegin() {
	stackFrame_t* stk; asm volatile("mov %%ebp, %0" : "=r" (stk));
	while (stk->ebp)
		stk = stk->ebp;

	return (uint32_t)stk;
}

void createStack(uint32_t newStart, uint32_t size, bool isKernel) {
	allocFramesReversed(newStart - size, newStart, 1, isKernel, 1);

	uint32_t pdAddr;
	asm volatile("mov %%cr3, %0" : "=r" (pdAddr));
	asm volatile("mov %0, %%cr3" : : "r" (pdAddr));
}

void cloneStack(uint32_t oldStart, uint32_t newStart, uint32_t size, bool isKernel, uint32_t* ESP, uint32_t* EBP) {
	createStack(newStart, size, isKernel);

	uint32_t oldESP = *ESP;
	uint32_t oldEBP = *EBP;

	uint32_t offset	= newStart - oldStart;

	uint32_t newESP	= oldESP + offset;
	uint32_t newEBP	= oldEBP + offset;

	memcpy((void*)newESP, (void*)oldESP, (uint32_t)oldStart - oldESP);

	stackFrame_t* stk = (stackFrame_t*)newEBP;
	
	while ((uint32_t)stk != newStart) {
		stk->ebp = (stackFrame_t*)((uint32_t)stk->ebp + offset);
		stk = stk->ebp;
	}
		
	*ESP = newESP;
	*EBP = newEBP;
}

void moveStack(uint32_t oldStart, uint32_t newStart, uint32_t size) {
	uint32_t esp;	asm volatile("mov %%esp, %0" : "=r" (esp));
	uint32_t ebp;	asm volatile("mov %%ebp, %0" : "=r" (ebp));

	cloneStack(oldStart, newStart, size, true, &esp, &ebp);

	asm volatile("mov %0, %%esp" : : "r" (esp));
	asm volatile("mov %0, %%ebp" : : "r" (ebp));
}

void yieldInterrupt(CPURegisters_t* regs) {
	if (!isTaskingInit())
		return;
	
	currentTask->status = TS_YIELD;
	switchTask(regs);
}

void yield() {
	asm volatile("int $0x40");
}

int32_t getPID() {
	return currentTask ? currentTask->id : -1;
}

Task_t* getCurrentTask() {
	return currentTask;
}

uint8_t	isTaskingInit() {
	return currentTask ? 1 : 0;
}

uint8_t getCPL() {
	return currentTask->regs.cs & 3;
}