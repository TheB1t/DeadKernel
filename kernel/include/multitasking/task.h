#pragma once

#include <utils/common.h>
#include <utils/stackTrace.h>
#include <memory_managment/heap.h>
#include <memory_managment/kheap.h>
#include <memory_managment/paging.h>
#include <interrupts/descriptor_tables.h>
#include <interrupts/isr.h>
#include <fs/elf/elf.h>

#define KERNEL_STACK_SIZE			(0x4000)

#define	PROCESS_STACK_SIZE			(0x4000)
#define BASE_PROCESS_ESP			(0xB0000000)

#define list_pass_entry(l, e) ((l)->next == (e)) ? (e)->next : (l)->next

#define list_for_each_forever(pos, head, root) \
    for (pos = list_pass_entry(head, root); (root)->next != (root) && (root)->prev != (root); \
    pos = list_pass_entry(pos, root))

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
	struct	list_head	list;
} Task_t;

typedef struct task_queue {
	struct	list_head	head;
} TaskQueue_t;

extern TaskQueue_t mainQueue;
extern Task_t* kernelTask;
extern Task_t* currentTask;

void		initTasking();

void		switchTask(CPURegisters_t* regs);

void		copyFromUser(void* ptr0, void* userPtr, uint32_t size);
void		copyToUser(void* userPtr, void* ptr0, uint32_t size);

Task_t*		makeTaskFromELF(ELF32Obj_t* hdr);

void		initQueue(TaskQueue_t* queue);
void		insertTask(TaskQueue_t* queue, Task_t* task);
void		cutTask(Task_t* task);

int32_t		runTask(Task_t* task);
void		stopTask(Task_t* task);
void 		destroyTask(Task_t* task);
Task_t*		allocTask();
void		freeTask(Task_t* task);

uint8_t		isTaskingInit();
Task_t*		getCurrentTask();
int32_t		getPID();
uint8_t		getCPL();