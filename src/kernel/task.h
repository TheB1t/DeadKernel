#pragma once

#include "common.h"
#include "kheap.h"
#include "paging.h"
#include "descriptor_tables.h"

#define KERNEL_STACK_SIZE (2048)

typedef struct task {
	int32_t			id;
	uint32_t		esp, ebp;
	uint32_t		eip;
	PageDir_t*		pageDir;
	uint32_t		kernelStack;
	struct task*	next;
} Task_t;

void	initTasking();
void	taskSwitch();
int32_t	fork();
void	moveStack(void* newStackStart, uint32_t size);
int32_t	getPID();
void	switchToUserMode();
