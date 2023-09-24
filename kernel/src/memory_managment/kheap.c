#include <memory_managment/kheap.h>

extern uint32_t end;
uint32_t placementAddress = (uint32_t)&end;

Heap_t* kernelHeap = 0;

uint32_t _kmalloc(uint32_t size, uint8_t align, uint32_t* phys) {
	if (kernelHeap) {
		return heap_malloc(kernelHeap, size, align, phys);
	} else {
		if (align == 1 && (placementAddress & 0xFFFFF000)) {
			placementAddress &= 0xFFFFF000;
			placementAddress += 0x1000;
		}
		if (phys) {
			*phys = placementAddress;
		}
		uint32_t tmp = placementAddress;
		placementAddress += size;
		return tmp;
	}
}

uint32_t kmalloc(uint32_t size) {
	return _kmalloc(size, 0, 0);
}

void kfree(void* addr) {
	heap_free(kernelHeap, addr);
}