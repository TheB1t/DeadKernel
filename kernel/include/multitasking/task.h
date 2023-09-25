#pragma once

#include <utils/common.h>
#include <utils/stackTrace.h>
#include <memory_managment/heap.h>
#include <memory_managment/kheap.h>
#include <memory_managment/paging.h>
#include <interrupts/descriptor_tables.h>
#include <interrupts/isr.h>
#include <fs/elf/elf.h>

#define KERNEL_STACK_SIZE			(2048)

#define	PROCESS_STACK_SIZE			(0x4000)
#define BASE_PROCESS_ESP			(0xB0000000)

#define QUEUE_TO_TASK(q)			((Task_t*)&q)
#define QUEUE_FIRST_TASK(q)			(QUEUE_TO_TASK(q)->next)

typedef enum {
	TS_CREATED	= 0,
	TS_IDLE		= 1,
	TS_RUNNING	= 2,
	TS_FINISHED	= 3,
	TS_YIELD	= 4,
	TS_STOPPED	= 5,
	TS_ZOMBIE   = 6
} Status_t;
		
typedef struct task {
	CPURegisters_t	regs;
	int32_t			id;
	PageDir_t*		pageDir;
	Status_t		status;
	uint32_t		entry;
	int32_t			exitcode;
	uint32_t		time;
	Heap_t*			heap;
	ELF32Obj_t*		elf_obj;
	struct task*	next;
	struct task*	prev;
} Task_t;

typedef Task_t TaskQueue_t;

extern Task_t* kernelTask;
extern Task_t* currentTask;

void		initTasking();
void		switchTask(CPURegisters_t* regs);
void		copyFromUser(void* ptr0, void* userPtr, uint32_t size);
void		copyToUser(void* userPtr, void* ptr0, uint32_t size);
Task_t*		makeTaskFromELF(ELF32Obj_t* hdr);
int32_t		runTask(Task_t* task);
void		stopTask(Task_t* task);
void 		destroyTask(Task_t* task);
Task_t*		allocTask();
void		freeTask(Task_t* task);
void		yield();
int32_t		getPID();
Task_t*		getCurrentTask();
uint8_t		isTaskingInit();
uint8_t		getCPL();
