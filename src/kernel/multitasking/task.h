#pragma once

#include "common.h"
#include "kheap.h"
#include "paging.h"
#include "descriptor_tables.h"
#include "isr.h"
#include "elf.h"
#include "serial.h"

#define KERNEL_STACK_SIZE			(2048)

#define	PROCESS_STACK_SIZE			(0x2000)
#define BASE_PROCESS_ESP			(0xB0000000)

#define QUEUE_TO_TASK(q)			((Task_t*)&q)
#define QUEUE_FIRST_TASK(q)			(QUEUE_TO_TASK(q)->next)

typedef enum {
	TS_CREATED	= 0,
	TS_IDLE		= 1,
	TS_RUNNING	= 2,
	TS_FINISHED	= 3,
	TS_YIELD	= 4,
	TS_STOPPED	= 5
} Status_t;
		
typedef struct task {
	CPURegisters_t	regs;			//0
	int32_t			id;				//40
	PageDir_t*		pageDir;		//44
	Status_t		status;			//48
	uint32_t		kernelStack;	//52
	uint32_t		entry;			//56
	int32_t			exitcode;
	struct task*	next;
	struct task*	prev;
} Task_t;

typedef Task_t TaskQueue_t;

void		initTasking();
void		switchTask(CPURegisters_t* regs);
Task_t*		makeTaskFromELF(ELF32Header_t* hdr, uint8_t makeUserProcess);
int32_t		runTask(Task_t* task);
void		stopTask(Task_t* task);
Task_t*		allocTask();
void		freeTask(Task_t* task);
void		yield();
int32_t		getPID();
Task_t*		getCurrentTask();
uint8_t		isTaskingInit();
uint8_t		getCPL();
