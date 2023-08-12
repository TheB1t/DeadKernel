#pragma once

#include <utils/common.h>
#include <memory_managment/paging.h>
#include <utils/llist.h>

#define KHEAP_START         0xC0000000
#define KHEAP_MIN_SIZE      0x100000

typedef struct {
	uint32_t			startAddr;
	uint32_t			endAddr;
	uint32_t			maxAddr;
	uint8_t				supervisor;
	uint8_t				readonly;
	struct	list_head	head;
} Heap_t;

typedef	struct {
	uint32_t			prevFoot;
	uint32_t			head;
	struct	list_head	list;
} HeapChunk_t;

Heap_t* createHeap(uint32_t placementAddr, uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly);
void*	alloc(uint32_t size, Heap_t* heap);
void*	palignedAlloc(uint32_t size, Heap_t* heap);
void	free(void* p, Heap_t* heap);

uint32_t	_kmalloc(uint32_t size, uint8_t align, uint32_t* phys);
uint32_t	kmalloc(uint32_t size);
void 		kfree(void* addr);
