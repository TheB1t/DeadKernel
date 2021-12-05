#pragma once

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "blockDevice.h"

#define nullptr (0)
#define GET(a) (*(a))

#define FS_MAGIC		0xDEADBEEF

#define FS_FLAG_BUSY	0b00000001
#define FS_FLAG_FILE	0b00000010
#define FS_FLAG_DIR		0b00000100

typedef struct {
	uint32_t	magic;
	uint32_t	blockSize;
	uint32_t	blockCount;
	uint32_t	inodesFirstBlock;
	uint32_t	uuid;
} initrdFS_Header_t;

typedef struct {
	uint32_t	number;
	uint8_t		flags;
	uint32_t	dataBlock;		
} initrdFS_INode_t;

typedef struct {
	char		name[128];
	uint32_t	INodeNumber;
} initrdFS_DirEntry_t;

typedef struct {
	uint32_t	magic;
	uint32_t	INodeNumber;
	uint8_t		flags;	
} initrdFS_BlockHeader_t;

typedef struct {
	uint32_t	magic;
	uint32_t	prevBlock;
	uint32_t	nextBlock;
} initrdFS_BlockFooter_t;

typedef struct {
	initrdFS_BlockHeader_t*	header;
	initrdFS_BlockFooter_t* footer;
	uint8_t*				dataPtr;
	uint32_t 				dataSize;
} initrdFS_Block_t;

typedef struct {
	BlockDevice_t*	device;
	uint8_t*		buffer;
} initrdFS_t;

void initrdFS_init(initrdFS_t** fs, BlockDevice_t* device) {
	GET(fs)			= (initrdFS_t*)kmalloc(sizeof(initrdFS_t));
	GET(fs)->device = device;
	GET(fs)->buffer	= (uint8_t*)kmalloc(device->blockSize);
}

void initrdFS_deInit(initrdFS_t** fs) {
	kfree(GET(fs)->buffer);
	kfree(GET(fs));

	GET(fs) = nullptr;
}

void initrdFS_readBlock(initrdFS_t* fs, uint32_t block) {
	BlockDevice_readBlock(fs->device, block, 0, fs->device->blockSize, fs->buffer);
}

void initrdFS_writeBlock(initrdFS_t* fs, uint32_t block) {
	BlockDevice_writeBlock(fs->device, block, 0, fs->device->blockSize, fs->buffer);
}

void initrdFS_initBlock(initrdFS_t* fs, uint32_t block) {
	initrdFS_readBlock(fs, block);
	initrdFS_BlockHeader_t* header = (initrdFS_BlockHeader_t*)fs->buffer;	
	initrdFS_BlockFooter_t* footer = (initrdFS_BlockFooter_t*)fs->buffer + fs->device->blockSize - sizeof(initrdFS_BlockFooter_t);	

	uint8_t* dataPtr	= fs->buffer + sizeof(initrdFS_BlockHeader_t);
	uint8_t dataSize	= fs->device->blockSize - sizeof(initrdFS_BlockFooter_t) - sizeof(initrdFS_BlockHeader_t);
	memset(dataPtr, 0, dataSize);
	
	header->magic		= FS_MAGIC;
	header->INodeNumber = 0;
	header->flags		= 0;

	footer->magic		= FS_MAGIC;
	footer->prevBlock	= 0;
	footer->nextBlock	= 0;
	 
	initrdFS_writeBlock(fs, block);
}

void initrdFS_initBlocks(initrdFS_t* fs, uint32_t startBlock, uint32_t endBlock) {
	for (uint32_t i = startBlock; i < endBlock; i++)
		initrdFS_initBlock(fs, i);
}

void initrdFS_buf2block(initrdFS_t* fs, initrdFS_Block_t* blockEntry) {
	blockEntry->header		= (initrdFS_BlockHeader_t*)fs->buffer;	
	blockEntry->footer		= (initrdFS_BlockFooter_t*)fs->buffer + fs->device->blockSize - sizeof(initrdFS_BlockFooter_t);	
	blockEntry->dataPtr		= fs->buffer + sizeof(initrdFS_BlockHeader_t);
	blockEntry->dataSize	= fs->device->blockSize - sizeof(initrdFS_BlockFooter_t) - sizeof(initrdFS_BlockHeader_t);
}

void initrdFS_initHeader(initrdFS_t* fs, uint32_t block, uint32_t blockSize, uint32_t blockCount, uint32_t inodesFirstBlock) {
	initrdFS_readBlock(fs, block);
	initrdFS_Header_t* header = (initrdFS_Header_t*)fs->buffer;
	
	header->magic				= FS_MAGIC;
	header->blockSize			= fs->device->blockSize;
	header->blockCount			= fs->device->blockCount;
	header->inodesFirstBlock	= inodesFirstBlock;
	header->uuid				= 0; //TODO where is unique id? mmm?
	
	initrdFS_writeBlock(fs, block);
}

void initrdFS_linkBlocks(initrdFS_t* fs, uint32_t block1, uint32_t block2) {
	initrdFS_Block_t* blockEntry = (initrdFS_Block_t*)kmalloc(sizeof(initrdFS_Block_t));

	uint32_t tmpBlock = 0;
	
	initrdFS_readBlock(fs, block1);
	initrdFS_buf2block(fs, blockEntry);

	if (blockEntry->footer->nextBlock)
		tmpBlock = blockEntry->footer->nextBlock;

	blockEntry->footer->nextBlock = block2;

	initrdFS_writeBlock(fs, block1);

	if (tmpBlock) {
		initrdFS_readBlock(fs, tmpBlock);
		initrdFS_buf2block(fs, blockEntry);

		blockEntry->footer->prevBlock = block2;

		initrdFS_writeBlock(fs, tmpBlock);	
	}

	initrdFS_readBlock(fs, block2);
	initrdFS_buf2block(fs, blockEntry);

	blockEntry->footer->prevBlock = block1;
	blockEntry->footer->nextBlock = tmpBlock;

	initrdFS_writeBlock(fs, block2);
	
	kfree(blockEntry);
}

uint32_t initrdFS_findFreeBlock(initrdFS_t* fs) {
	uint32_t block = 0;
	
	initrdFS_Block_t* blockEntry = (initrdFS_Block_t*)kmalloc(sizeof(initrdFS_Block_t));
	for (uint32_t i = 1; i < fs->device->blockCount; i++) {
		initrdFS_readBlock(fs, block);		
		initrdFS_buf2block(fs, blockEntry);
		
		if (!(blockEntry->header->flags & FS_FLAG_BUSY)) {
			block = i;
			break;
		}
	}

	kfree(blockEntry);
	return block;
}

void initrdFS_initRootNode(initrdFS_t* fs, uint32_t block) {
	initrdFS_readBlock(fs, block);
	initrdFS_Block_t* blockEntry = (initrdFS_Block_t*)kmalloc(sizeof(initrdFS_Block_t));
	initrdFS_buf2block(fs, blockEntry);

	blockEntry->header->flags = FS_FLAG_BUSY;
	
	initrdFS_INode_t* inode0 = (initrdFS_INode_t*)blockEntry->dataPtr;
	
	inode0->number	= 0;
	inode0->flags	= FS_FLAG_DIR | FS_FLAG_BUSY;
	
	initrdFS_writeBlock(fs, block);

	kfree(blockEntry);
}

uint8_t initrdFS_findFS(initrdFS_t* fs) {
	initrdFS_readBlock(fs, 0);
	initrdFS_Header_t* header = (initrdFS_Header_t*)fs->buffer;
	if (header->magic == FS_MAGIC)
		return 1;

	return 0;
}	

void initrdFS_makeFS(initrdFS_t* fs) {
	initrdFS_initHeader(fs, 0, 512, 1024, 1);
	initrdFS_initBlocks(fs, 1, fs->device->blockCount);
	initrdFS_initRootNode(fs, 1);
}
