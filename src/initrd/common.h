#pragma once
#include <stdlib.h>

typedef	unsigned int	uint32_t;
typedef	unsigned short	uint16_t;
typedef	unsigned char	uint8_t;

uint32_t alignValue(uint32_t value, uint32_t base) {
    uint32_t tmpBase = ~(0xFFFFFFFF % base);
    if (value & tmpBase) {
        value &= tmpBase;
        value += base;
    }
    return value;
}

static inline void* _kmalloc(uint32_t size, uint8_t align, uint32_t* phys) {
	return malloc(size);
}

static inline void* kmalloc(uint32_t size) {
	return _kmalloc(size, 0, 0);
}

static inline void kfree(void* ptr) {
	free(ptr);
}
