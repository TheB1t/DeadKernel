#pragma once

#include <stdlib.h>
#include <string.h>
#include "common.h"

#define nullptr (0)
#define GET(a) (*(a))

#define BLOCK2ADDR(d, b)	((d)->blockSize * (b))
#define ADDR2BLOCK(d, a)	((b) / (d)->blockSize)
#define BLOCK2PTR(d, b)		((d)->start + BLOCK2ADDR(d, b))
#define PTR2BLOCK(d, p)		(((p) - (d)->start) / (d)->blockSize)

#define FLAG_FREEABLE	0b00000001

typedef struct {
	uint8_t*	start;
	uint8_t*	buffer;
	uint8_t		flags;
	uint32_t	blockSize;
	uint32_t	blockCount;
} BlockDevice_t;


void BlockDevice_init(BlockDevice_t** device, uint32_t blockSize, uint32_t blockCount) {
	GET(device)				= (BlockDevice_t*)kmalloc(sizeof(BlockDevice_t));
	
	GET(device)->start		= (uint8_t*)kmalloc(blockSize * blockCount);
	GET(device)->buffer		= (uint8_t*)kmalloc(blockSize);
	GET(device)->blockSize	= blockSize;
	GET(device)->blockCount	= blockCount;
	GET(device)->flags		= FLAG_FREEABLE;
}

void BlockDevice_initByPtr(BlockDevice_t** device, uint32_t blockSize, uint32_t blockCount, uint8_t* ptr) {
	GET(device)				= (BlockDevice_t*)kmalloc(sizeof(BlockDevice_t));

	GET(device)->start		= ptr;
	GET(device)->buffer		= (uint8_t*)kmalloc(blockSize);
	GET(device)->blockSize	= blockSize;
	GET(device)->blockCount	= blockCount;
	GET(device)->flags		= 0;	
}

uint32_t BlockDevice_readBlock(BlockDevice_t* device, uint32_t block, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (size > device->blockSize)
		return 0;
		
	if (offset + size > device->blockSize)
		size = device->blockSize - offset;
			
	memcpy(buffer, BLOCK2PTR(device, block) + offset, size);
	return size;
}

uint32_t BlockDevice_writeBlock(BlockDevice_t* device, uint32_t block, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (size > device->blockSize)
		return 0;
		
	if (offset + size > device->blockSize)
		size = device->blockSize - offset;
			
	memcpy(BLOCK2PTR(device, block) + offset, buffer, size);
	return size;
}

void BlockDevice_deInit(BlockDevice_t** device) {
	if (GET(device)->flags & FLAG_FREEABLE)
		kfree(GET(device)->start);
	kfree(GET(device)->buffer);
	kfree(GET(device));

	GET(device) = nullptr;
}
