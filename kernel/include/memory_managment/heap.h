#pragma once

#include <utils/common.h>
#include <memory_managment/paging.h>
#include <llist.h>

#define HEAP_START          0x90000000
#define HEAP_MIN_SIZE       0x100000

typedef struct {
	uint32_t			startAddr;
	uint32_t			endAddr;
	uint32_t			maxAddr;
	uint8_t				supervisor;
	uint8_t				readonly;
	PageDir_t*			pageDir;
	struct	list_head	head;
} Heap_t;

typedef	struct {
	uint32_t			prevFoot;
	uint32_t			head;
	struct	list_head	list;
} HeapChunk_t;

Heap_t*     createHeap(uint32_t placementAddr, PageDir_t* pageDir, uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly);
void*	    alloc(uint32_t size, Heap_t* heap);
void*	    palignedAlloc(uint32_t size, Heap_t* heap);
void        free(void* p, Heap_t* heap);

uint32_t	heap_malloc(Heap_t* heap, uint32_t size, uint8_t align, uint32_t* phys);
void		heap_free(Heap_t* heap, void* addr);