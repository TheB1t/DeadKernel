#pragma once

#include "common.h"
#include "kheap.h"
#include "paging.h"
#include "descriptor_tables.h"
#include "isr.h"
#include "elf.h"

#define KERNEL_STACK_SIZE (2048)

#define	PROCESS_STACK_SIZE			(0x2000)
#define BASE_PROCESS_ESP			(0xB0000000)
#define BASE_PROCESS_EBP			(0xB0000000)

typedef enum {
	TS_IDLE		= 0,
	TS_RUNNING	= 1,
	TS_FINISHED	= 2,
	TS_YIELD	= 3
} Status_t;
		
typedef struct task {
	CPURegisters_t	regs;			//0
	int32_t			id;				//40
	PageDir_t*		pageDir;		//44
	Status_t		status;			//48
	uint32_t		kernelStack;	//52
	uint32_t		entry;			//56
	struct task*	next;
	struct task*	prev;
} Task_t;

void		initTasking();
void		yield();
void		switchTask(CPURegisters_t* regs);
int32_t		runTask(Task_t* task);
Task_t*		stopTask(Task_t* task);
Task_t*		createTask(uint32_t esp, uint32_t ebp, uint32_t entry, PageDir_t* dir);
int32_t		freeTask(Task_t* task);
Task_t*		makeTaskFromELF(ELF32Header_t* hdr);
int32_t		getPID();
Task_t*		getCurrentTask();
uint8_t		isTaskingInit();
void		switchToUserMode();
