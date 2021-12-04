#include "task.h"

typedef struct stackFrame {
	struct stackFrame* ebp;
	uint32_t eip;
} stackFrame_t;

Task_t* tempTask = 0;
Task_t* currentTask = 0;
Task_t* queue = 0;

extern PageDir_t* kernelDir;
extern PageDir_t* currentDir;

uint32_t nextPID = 1;

uint32_t getStackBegin() {
	stackFrame_t* stk; asm volatile("mov %%ebp, %0" : "=r" (stk));
	while (stk->ebp)
		stk = stk->ebp;

	return (uint32_t)stk;
}

void taskHalted(uint32_t returnValue) {
	if (queue) {
		currentTask->status = TS_FINISHED;
		//asm volatile("sti");
		printf("PID %d exit with code 0x%08x\n", currentTask->id, returnValue);
	}
}

extern void loadEntry();
/*
void loadEntry() {
	int32_t (*entry)() = (int32_t (*)())currentTask->entry;
	int32_t returnValue = entry();

	currentTask->status = TS_FINISHED;
	printf("PID %d exit with code 0x%08x\n", currentTask->id, returnValue);
//	asm volatile("sti");
	for(;;);
}
*/

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

void initTasking() {
	asm volatile("cli");
	
	moveStack(getStackBegin(), 0xE0000000, 0x2000);

	queue					= (Task_t*)kmalloc(sizeof(Task_t));
	memset((uint8_t*)queue, 0, sizeof(Task_t));
	
	currentTask					= queue;
	currentTask->id				= nextPID++;
	currentTask->pageDir		= currentDir;
	currentTask->kernelStack	= 0;//_kmalloc(KERNEL_STACK_SIZE, 1, 0);
	currentTask->status			= TS_IDLE;

	registerInterruptHandler(64, &switchTask);
	asm volatile("sti");
}

extern void saveContext(TaskRegisters_t*);
extern void loadContext(TaskRegisters_t*);
void switchContext(Task_t* from, Task_t* to, uint32_t physicalAddr) {
	saveContext(&from->regs);
	loadContext(&to->regs);
	asm volatile("mov %0, %%cr3" : : "r" (physicalAddr));
}

void yield() {
	currentTask->status = TS_YIELD;
	asm volatile("int $0x40");
}

void switchTask() {	
	if (!currentTask)
		return;

	tempTask = currentTask;
	while (1) {
		switch (currentTask->status) {
			case TS_YIELD:
			case TS_RUNNING:
				currentTask->status = TS_IDLE;
				break;

			case TS_IDLE:
				currentTask->status = TS_RUNNING;
				break;

			case TS_FINISHED:
				if (currentTask == queue) {
					if (currentTask->next)
						queue = currentTask->next;
					else
						PANIC("Last task finished");
				}
				
				stopTask(currentTask);
				break;			
		}

		if (currentTask->status == TS_RUNNING)
			break;
			
		currentTask = currentTask->next ? currentTask->next : queue;
	}

	currentDir = currentTask->pageDir;

	setKernelStack(currentTask->kernelStack + KERNEL_STACK_SIZE);
	switchContext(tempTask, currentTask, currentDir->physicalAddr);
}

Task_t* makeTaskFromELF(ELF32Header_t* hdr) {
	asm volatile("cli");

	PageDir_t* clonedDir = cloneDir(currentDir);
	
	Task_t* newTask			= (Task_t*)kmalloc(sizeof(Task_t));
	memset((uint8_t*)newTask, 0, sizeof(Task_t));
	newTask->pageDir		= clonedDir;
	newTask->kernelStack	= queue->kernelStack;//_kmalloc(KERNEL_STACK_SIZE, 1, 0);
	newTask->status			= TS_IDLE;
	newTask->regs.eax		= hdr->entry;
	newTask->regs.eip		= (uint32_t)loadEntry;
	newTask->regs.esp		= BASE_PROCESS_ESP;
	newTask->regs.ebp		= BASE_PROCESS_EBP;

	PageDir_t* savedDir = switchPageDir(clonedDir);
		
//	createStack(BASE_PROCESS_ESP, PROCESS_STACK_SIZE);
	
	for (uint32_t i = 0; i < hdr->phnum; i++) {
		ELF32ProgramHeader_t* ph = ELFProgram(hdr, i);
		if (ph->type == PT_LOAD) {
			allocFrames(ph->vaddr, ph->vaddr + ph->memsz, 1, 0, 1);
			memcpy((uint8_t*)ph->vaddr, ((uint8_t*)hdr) + ph->offset, ph->filesz);
		}
	}

	switchPageDir(savedDir);
	
	asm volatile("sti");
	return newTask;
}

int32_t runTask(Task_t* task) {
	asm volatile("cli");
	
	task->id = task->id ? task->id : nextPID++;

	tempTask = (Task_t*)queue;
	while (tempTask->next)
		tempTask = tempTask->next;

	tempTask->next = task;
	task->prev = tempTask;
	
	asm volatile("sti");
	return task->id;	
}

Task_t* stopTask(Task_t* task) {
	asm volatile("cli");
	
	if (task->prev)
		task->prev->next = task->next;
					
	if (task->next)
		task->next->prev = task->prev;
				
	asm volatile("sti");	
	return task->next;
}

int32_t getPID() {
	return currentTask->id;
}

Task_t* getCurrentTask() {
	return currentTask;
}

uint8_t	isTaskingInit() {
	return queue ? 1 : 0;
}

void switchToUserMode() {
	printf("Switching to user-mode...\n");
	setKernelStack(currentTask->kernelStack + KERNEL_STACK_SIZE);

   asm volatile("		\
     cli;				\
     mov $0x23, %ax;	\
     mov %ax, %ds;		\
     mov %ax, %es;		\
     mov %ax, %fs;		\
     mov %ax, %gs;		\
						\
     mov %esp, %eax;	\
     pushl $0x23;		\
     pushl %eax;		\
     pushf;				\
     pushl $0x1B;		\
     push $1f;			\
     iret;				\
   1:					\
						");	
}
