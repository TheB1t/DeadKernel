#include "task.h"

typedef struct stackFrame {
	struct stackFrame* ebp;
	uint32_t eip;
} stackFrame_t;

Task_t* kernelTask		= 0;
Task_t* currentTask		= 0;

TaskQueue_t mainQueue;

extern PageDir_t* kernelDir;
extern PageDir_t* currentDir;
extern void loadEntry();

uint32_t nextPID = 0;

void		insertTask(TaskQueue_t* queue, Task_t* task);
void		cutTask(Task_t* task);
void		initQueue(TaskQueue_t* queue);
uint32_t	getStackBegin();
void		createStack(uint32_t newStart, uint32_t size);
void		cloneStack(uint32_t oldStart, uint32_t newStart, uint32_t size, uint32_t* ESP, uint32_t* EBP);
void		moveStack(uint32_t oldStart, uint32_t newStart, uint32_t size);
void		yieldInterrupt(CPURegisters_t* regs);

void initTasking() {
	DISABLE_INTERRUPTS;
	
	// globalMutex = mutex_alloc();
	initQueue(&mainQueue);

	moveStack(getStackBegin(), 0xE0000000, 0x2000);

	kernelTask					= allocTask();
	
	kernelTask->pageDir			= currentDir;
	kernelTask->kernelStack		= _kmalloc(KERNEL_STACK_SIZE, 1, 0);
	kernelTask->status			= TS_CREATED;
	kernelTask->time			= 0;

	kernelTask->regs.cs			= GDT_DESC_SEG(GDT_DESC_KERNEL_CODE, PL_RING0);
	kernelTask->regs.ds			= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0);
	kernelTask->regs.ss0		= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0);
	kernelTask->regs.cr3		= currentDir->physicalAddr;
	kernelTask->regs.eflags		= 0x200;
	asm volatile("				\
				  mov %%cs, %0;	\
				  mov %%ds, %1;	\
				":"=r" (kernelTask->regs.cs),
				  "=r" (kernelTask->regs.ds));

	runTask(kernelTask);

	registerInterruptHandler(64, &yieldInterrupt);
	ENABLE_INTERRUPTS;
}

static void copyContext(CPURegisters_t* dst, CPURegisters_t* src) {
	#define COPY(dst, src, field)	(dst)->field = (src)->field

	COPY(dst, src, eax);
	COPY(dst, src, ebx);
	COPY(dst, src, ecx);
	COPY(dst, src, edx);
	COPY(dst, src, esi);
	COPY(dst, src, edi);	

	COPY(dst, src, ebp);
	COPY(dst, src, cr3);
	COPY(dst, src, ds);

	// First level IRET context
	serialprintf(COM1, "[copyContext] EIP 0x%08x CS 0x%08x EFLAGS 0x%08x\n", src->eip, src->cs, src->eflags);
	COPY(dst, src, eip);
	COPY(dst, src, cs);
	COPY(dst, src, eflags);

	if ((src->cs & 3) > 0) {
		// Second level IRET context
		serialprintf(COM1, "[copyContext] SS0 0x%08x ESP0 0x%08x\n", src->ss0, src->esp0);
		COPY(dst, src, ss0);
		COPY(dst, src, esp0);
	}

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
					stopTask(currentTask);
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

	setKernelStack(currentTask->kernelStack + KERNEL_STACK_SIZE);

	int oldCPL = oldTask->regs.cs & 3;
	int newCPL = currentTask->regs.cs & 3;

	if (oldCPL != newCPL)
		serialprintf(COM1, "[switchTask] Privelege %s %d -> %d\n", newCPL < oldCPL ? "escalation" : "downgrading", oldCPL, newCPL);

	copyContext(regs, &currentTask->regs);

	serialprintf(COM1, "[switchTask] Do switching PID %d -> PID %d\n", oldTask->id, currentTask->id);
	BREAKPOINT;
}

Task_t* makeTaskFromELF(ELF32Header_t* hdr, uint8_t makeUserProcess) {
	DISABLE_INTERRUPTS;
	PageDir_t* clonedDir	= cloneDir(currentDir);
	
	Task_t* newTask			= allocTask();

	newTask->pageDir		= clonedDir;
	newTask->kernelStack	= _kmalloc(KERNEL_STACK_SIZE, 1, 0);
	newTask->status			= TS_CREATED;
	newTask->time			= 0;
	newTask->regs.eax		= hdr->entry;
	newTask->regs.eip		= (uint32_t)loadEntry;
	newTask->regs.esp0		= BASE_PROCESS_ESP;

	newTask->regs.cr3		= clonedDir->physicalAddr;
	if (makeUserProcess) {
		newTask->regs.cs	= GDT_DESC_SEG(GDT_DESC_USER_CODE, PL_RING3);
		newTask->regs.ds	= GDT_DESC_SEG(GDT_DESC_USER_DATA, PL_RING3);
		newTask->regs.ss0   = GDT_DESC_SEG(GDT_DESC_USER_DATA, PL_RING3);
	} else {
		newTask->regs.cs	= GDT_DESC_SEG(GDT_DESC_KERNEL_CODE, PL_RING0);
		newTask->regs.ds	= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0);	
		newTask->regs.ss0   = GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0);
	}
	newTask->regs.eflags	= 0x200;

	PageDir_t* savedDir = switchPageDir(clonedDir);

	createStack(BASE_PROCESS_ESP, PROCESS_STACK_SIZE);
	
	for (uint32_t i = 0; i < hdr->phnum; i++) {
		ELF32ProgramHeader_t* ph = ELFProgram(hdr, i);
		if (ph->type == PT_LOAD) {
			allocFrames(ph->vaddr, ph->vaddr + ph->memsz, 1, 0, 1);
			memcpy((uint8_t*)ph->vaddr, ((uint8_t*)hdr) + ph->offset, ph->filesz);
		}
	}

	switchPageDir(savedDir);
	
	ENABLE_INTERRUPTS;
	return newTask;
}

int32_t runTask(Task_t* task) {
	if (task->status != TS_CREATED || task->status == TS_STOPPED)
		return -1;

	DISABLE_INTERRUPTS;
	
	task->id = task->id ? task->id : nextPID++;

	insertTask(&mainQueue, task);

	task->status = TS_IDLE;

	if (!currentTask)
		currentTask = task;

	ENABLE_INTERRUPTS;
	return task->id;	
}

void stopTask(Task_t* task) {
	if (task->status == TS_CREATED)
		return;

	DISABLE_INTERRUPTS;
	
	cutTask(task);
	
	if (task->status != TS_FINISHED)
		task->status == TS_STOPPED;
	
	ENABLE_INTERRUPTS;
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
	kfree((uint32_t)task);
}

uint32_t getStackBegin() {
	stackFrame_t* stk; asm volatile("mov %%ebp, %0" : "=r" (stk));
	while (stk->ebp)
		stk = stk->ebp;

	return (uint32_t)stk;
}

void createStack(uint32_t newStart, uint32_t size) {
	for (uint32_t i = newStart; i >= newStart - size; i -= PAGE_SIZE)
		allocFrame(getPage(i, 1, currentDir), 0, 1);

	uint32_t pdAddr;
	asm volatile("mov %%cr3, %0" : "=r" (pdAddr));
	asm volatile("mov %0, %%cr3" : : "r" (pdAddr));
}

void cloneStack(uint32_t oldStart, uint32_t newStart, uint32_t size, uint32_t* ESP, uint32_t* EBP) {
	createStack(newStart, size);

	uint32_t oldESP = *ESP;
	uint32_t oldEBP = *EBP;

	uint32_t offset	= newStart - oldStart;

	uint32_t newESP	= oldESP + offset;
	uint32_t newEBP	= oldEBP + offset;

	memcpy((void*)newESP, (void*)oldESP, (uint32_t)oldStart - oldESP);

	stackFrame_t* stk = (stackFrame_t*)newEBP;
	
	while (stk->ebp) {
		stk->ebp = (stackFrame_t*)((uint32_t)stk->ebp + offset);
		stk = stk->ebp;
	}
		
	*ESP = newESP;
	*EBP = newEBP;
}

void moveStack(uint32_t oldStart, uint32_t newStart, uint32_t size) {
	uint32_t esp;	asm volatile("mov %%esp, %0" : "=r" (esp));
	uint32_t ebp;	asm volatile("mov %%ebp, %0" : "=r" (ebp));

	cloneStack(oldStart, newStart, size, &esp, &ebp);

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