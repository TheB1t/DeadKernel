#include "task.h"

volatile Task_t*	currentTask;
volatile Task_t*	readyQueue;

extern	PageDir_t*	kernelDir;
extern	PageDir_t*	currentDir;
extern	uint32_t	initialESP;

extern	void		allocFrame(Page_t*, uint32_t, uint32_t);
extern	void		taskWakeup(uint32_t, uint32_t, uint32_t, uint32_t);
extern	uint32_t	readEIP();

uint32_t nextPID = 1;

void initTasking() {
	asm volatile("cli");

	currentDir = cloneDir(kernelDir);
	switchPageDir(currentDir);	

	moveStack((void*)0xE0000000, 0x2000);

	currentTask					= readyQueue = (Task_t*)kmalloc(sizeof(Task_t));
	currentTask->id				= nextPID++;
	currentTask->esp			= currentTask->ebp = 0;
	currentTask->eip			= 0;
	currentTask->pageDir		= currentDir;
	currentTask->kernelStack	= _kmalloc(KERNEL_STACK_SIZE, 1, 0);
	currentTask->next			= 0;

	asm volatile("sti");
}

void moveStack(void* newStackStart, uint32_t size) {
	for (uint32_t i = (uint32_t)newStackStart; i >= (uint32_t)(newStackStart - size); i -= PAGE_SIZE)
		allocFrame(getPage(i, 1, currentDir), 0, 1);

	uint32_t pdAddr;
	asm volatile("mov %%cr3, %0" : "=r" (pdAddr));
	asm volatile("mov %0, %%cr3" : : "r" (pdAddr));

	uint32_t oldStackPointer;	asm volatile("mov %%esp, %0" : "=r" (oldStackPointer));
	uint32_t oldBasePointer;	asm volatile("mov %%ebp, %0" : "=r" (oldBasePointer));

	uint32_t offset				= (uint32_t)newStackStart - initialESP;

	uint32_t newStackPointer	= oldStackPointer + offset;
	uint32_t newBasePointer		= oldBasePointer + offset;

	memcpy((void*)newStackPointer, (void*)oldStackPointer, initialESP - oldStackPointer);

	for (uint32_t i = (uint32_t)newStackStart; i > (uint32_t)newStackStart - size; i -= 4) {
		uint32_t tmp = *(uint32_t*)i;

		if ((oldStackPointer < tmp) && (tmp	< initialESP)) {
			tmp = tmp + offset;
			uint32_t *tmp2 = (uint32_t*)i;
			*tmp2 = tmp;
		}
	}

	asm volatile("mov %0, %%esp" : : "r" (newStackPointer));
	asm volatile("mov %0, %%ebp" : : "r" (newBasePointer));
}

void taskSwitch() {
	if (!currentTask)
		return;
	
	uint32_t esp;	asm volatile("mov %%esp, %0" : "=r" (esp));
	uint32_t ebp;	asm volatile("mov %%ebp, %0" : "=r" (ebp));
	uint32_t eip = readEIP();
	
	if (eip	== 0x12345)
		return;
	
	currentTask->eip = eip;
	currentTask->esp = esp;
	currentTask->ebp = ebp;

	currentTask = currentTask->next;

	if (!currentTask)
		currentTask = readyQueue;

	eip = currentTask->eip;
	esp = currentTask->esp;
	ebp = currentTask->ebp;

	currentDir = currentTask->pageDir;

	setKernelStack(currentTask->kernelStack + KERNEL_STACK_SIZE);
	
	taskWakeup(eip, esp, ebp, currentDir->physicalAddr);
}

int32_t fork() {
	asm volatile("cli");

	Task_t* parentTask = (Task_t*)currentTask;

	PageDir_t* directory = cloneDir(currentDir);

	Task_t* newTask = (Task_t*)kmalloc(sizeof(Task_t));

	newTask->id			= nextPID++;
	newTask->esp		= newTask->ebp = 0;
	newTask->eip		= 0;
	newTask->pageDir	= directory;
	currentTask->kernelStack	= _kmalloc(KERNEL_STACK_SIZE, 1, 0);
	newTask->next		= 0;

	Task_t* tmpTask = (Task_t*)readyQueue;
	while (tmpTask->next)
		tmpTask = tmpTask->next;

	tmpTask->next = newTask;
	
	uint32_t eip = readEIP();

	if (currentTask == parentTask) {
		uint32_t esp;	asm volatile("mov %%esp, %0" : "=r" (esp));
		uint32_t ebp;	asm volatile("mov %%ebp, %0" : "=r" (ebp));

		newTask->esp = esp;
		newTask->ebp = ebp;
		newTask->eip = eip;
		asm volatile("sti");

		return newTask->id;
	} else {
		asm volatile("sti");
		return 0;
	}
}

int32_t getPID() {
	return currentTask->id;
}

void switchToUserMode() {
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
