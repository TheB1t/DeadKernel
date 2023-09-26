#include <multitasking/task.h>
#include <multitasking/semaphore.h>

typedef struct stackFrame {
	struct stackFrame* ebp;
	uint32_t eip;
} stackFrame_t;

Task_t* kernelTask		= 0;
Task_t* currentTask		= 0;

TaskQueue_t mainQueue;
semaphore_queue_t semaphoreQueue;
uint32_t nextPID = 0;

uint32_t	getStackBegin();
void		createStack(uint32_t newStart, uint32_t size, bool isKernel);
void		cloneStack(uint32_t oldStart, uint32_t newStart, uint32_t size, bool isKernel, uint32_t* ESP, uint32_t* EBP);
void		moveStack(uint32_t oldStart, uint32_t newStart, uint32_t size);

void initTasking() {
	DISABLE_INTERRUPTS();
	
	INIT_LIST_HEAD(LIST_GET_HEAD(&mainQueue));
	semctl_init(&semaphoreQueue);

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

	ENABLE_INTERRUPTS();
}

void switchTask(CPURegisters_t* regs) {
	if (!isTaskingInit())
			return;

	memcpy(&currentTask->regs, regs, sizeof(CPURegisters_t));

	semctl_process();

	struct list_head* iter;
	list_for_each_forever(iter, LIST_GET_LIST(currentTask), LIST_GET_HEAD(&mainQueue)) {
		Task_t* task = list_entry(iter, Task_t, list);

		switch (task->status) {
			case TS_YIELD:
			case TS_RUNNING:
				if (task->regs.ecx == 0xFE11DEAD) {
					task->regs.ecx = 0;
					task->status = TS_ZOMBIE;
					task->exitcode = task->regs.eax;
					LOG_INFO("PID %d finished with exitcode 0x%08x in Ring %d", task->id, task->exitcode, task->regs.cs & 3);
				} else {
					task->status = TS_IDLE;
				}
				break;

			case TS_IDLE:
				task->status = TS_RUNNING;
				break;

			case TS_ZOMBIE:
			case TS_FINISHED:
				if (task == kernelTask) {
					if (list_empty(LIST_GET_HEAD(&mainQueue))) {
						LOG_INFO("Last task finished. Kernel be halted soon ;)");
						//Set self-destruct settings
						task->regs.ebx = 0x50000000;
						task->status = TS_RUNNING;
					}
				} else {
					destroyTask(task);
				}
				break;			
		}

		if (task->status == TS_RUNNING) {
			currentTask = task;
			break;
		}
	}

	setKernelStack(kernelTask->regs._esp);
	memcpy(regs, &currentTask->regs, sizeof(CPURegisters_t));
}

void copyFromUser(void* ptr0, void* userPtr, uint32_t size) {
	DISABLE_INTERRUPTS();

	PageDir_t* savedDir = switchPageDir(currentTask->pageDir);

	memcpy(ptr0, userPtr, size);

	switchPageDir(savedDir);

	ENABLE_INTERRUPTS();
}

void copyToUser(void* userPtr, void* ptr0, uint32_t size) {
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

			uint32_t start_appendix = section->addr & (PAGE_SIZE - 1);
			uint32_t end_appendix = (section->addr + section->size) & (PAGE_SIZE - 1);

			if (start_appendix)
				start -= PAGE_SIZE;

			if (end_appendix)
				end += PAGE_SIZE;

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

	if (!currentTask)
		currentTask = task;

	task->status = TS_IDLE;

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
	list_add_tail(LIST_GET_LIST(task), LIST_GET_HEAD(queue));
}

void cutTask(Task_t* task) {
	__list_del_entry(LIST_GET_LIST(task));
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


int32_t fork() {
	if (!isTaskingInit())
		return -1;

	CPURegisters_t* regs = getInterruptedContext();

	Task_t* newTask			= allocTask();

	newTask->elf_obj		= currentTask->elf_obj;
	newTask->status			= TS_CREATED;
	newTask->time			= 0;
	newTask->heap			= currentTask->heap;
	memcpy(&newTask->regs, regs, sizeof(CPURegisters_t));

	PageDir_t* clonedDir	= cloneDir(currentTask->pageDir);
		
	newTask->pageDir	= clonedDir;
	newTask->regs.cr3	= clonedDir->physicalAddr;
	newTask->regs.eax	= 0;

	runTask(newTask);

	serialprintf(COM1, "(0x%08x) Current task: %d (0x%08x), New task: %d (0x%08x)\n", regs->eip, currentTask->id, currentTask, newTask->id, newTask);

	return newTask->id;
}

void yield() {
	if (!isTaskingInit())
		return;
	
	currentTask->status = TS_YIELD;
	switchTask(getInterruptedContext());
}

uint8_t	isTaskingInit() {
	return nextPID ? 1 : 0;
}

inline Task_t* getCurrentTask() {
	return currentTask;
}

int32_t getPID() {
	return nextPID ? currentTask->id : -1;
}

uint8_t getCPL() {
	return currentTask->regs.cs & 3;
}