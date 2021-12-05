#pragma once

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "blockDevice.h"

#define nullptr (0)
#define GET(a) (*(a))

//.c file defines
#define READ_FLAGS(v, f)		(((v) & (f)) == (f))
#define PTR2HEADER(p)			((initrdFS_Header_t*)(p))
#define PTR2INODE(p)			((initrdFS_INode_t*)(p))
#define PTR2INODEN(p, n)		(PTR2INODE((p) + (sizeof(initrdFS_INode_t) * n)))
#define PTR2BHEADER(p)			((initrdFS_BlockHeader_t*)(p))
#define PTR2BFOOTER(p)			((initrdFS_BlockFooter_t*)(p))
#define OFFSET(p, o)			((p) + (o))
#define OFFSETBHEADER(p) 		(OFFSET(p, sizeof(initrdFS_BlockHeader_t)))
#define BLOCKDATASIZE(p) 		((p) - sizeof(initrdFS_BlockHeader_t) - sizeof(initrdFS_BlockFooter_t))
//end

#define FS_MAGIC		0xDEADBEEF

#define FS_FLAG_BUSY	0b00000001
#define FS_FLAG_FILE	0b00000010
#define FS_FLAG_DIR		0b00000100

typedef struct {
	uint32_t	magic;
	uint32_t	blockSize;
	uint32_t	blockCount;
	uint32_t	inodesFirstBlock;
	uint32_t	inodesPerBlock;
	uint32_t	firstFreeBlock;
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
	BlockDevice_t*		device;
	uint8_t*			buffer;
	initrdFS_Block_t*	blockEntry;
} initrdFS_t;

void initrdFS_init(initrdFS_t** fs, BlockDevice_t* device) {
	GET(fs)				= (initrdFS_t*)kmalloc(sizeof(initrdFS_t));
	GET(fs)->device 	= device;
	GET(fs)->buffer		= (uint8_t*)kmalloc(device->blockSize);
	GET(fs)->blockEntry = (initrdFS_Block_t*)kmalloc(sizeof(initrdFS_Block_t));
}

void initrdFS_deInit(initrdFS_t** fs) {
	kfree(GET(fs)->blockEntry);
	kfree(GET(fs)->buffer);
	kfree(GET(fs));

	GET(fs) = nullptr;
}

void initrdFS_readBlock(initrdFS_t* fs, uint32_t block) {
	BlockDevice_readBlock(fs->device, block, 0, fs->device->blockSize, fs->buffer);
	fs->blockEntry->header		= PTR2BHEADER(fs->buffer);	
	fs->blockEntry->footer		= PTR2BFOOTER(fs->buffer + fs->device->blockSize - sizeof(initrdFS_BlockFooter_t));	
	//printf("TEST %d %d\n", fs->device->blockSize, (void*)fs->blockEntry->footer - (void*)fs->blockEntry->header);
	fs->blockEntry->dataPtr		= OFFSETBHEADER(fs->buffer);
	fs->blockEntry->dataSize	= BLOCKDATASIZE(fs->device->blockSize);
}

void initrdFS_writeBlock(initrdFS_t* fs, uint32_t block) {
	BlockDevice_writeBlock(fs->device, block, 0, fs->device->blockSize, fs->buffer);
}

void initrdFS_pushBlock(initrdFS_t* fs, uint32_t baseBlock, uint32_t blockToPut) {
	uint32_t tmpBlock	= 0;
	uint32_t tmpOwn		= 0;
	
	//Edit base block
	initrdFS_readBlock(fs, baseBlock);

	if (fs->blockEntry->footer->nextBlock) 
		tmpBlock = fs->blockEntry->footer->nextBlock;

	tmpOwn = fs->blockEntry->header->INodeNumber;

	fs->blockEntry->footer->nextBlock = blockToPut;

	initrdFS_writeBlock(fs, baseBlock);

	//If old next block of base block exists, change prev block to blockToPut
	if (tmpBlock) {
		initrdFS_readBlock(fs, tmpBlock);

		fs->blockEntry->footer->prevBlock = blockToPut;

		initrdFS_writeBlock(fs, tmpBlock);	
	}

	//Edit block to put
	initrdFS_readBlock(fs, blockToPut);

	fs->blockEntry->footer->prevBlock = baseBlock;
	fs->blockEntry->footer->nextBlock = tmpBlock;
	fs->blockEntry->header->INodeNumber = tmpOwn;

	initrdFS_writeBlock(fs, blockToPut);
}

uint32_t initrdFS_popBlock(initrdFS_t* fs, uint32_t blockToPop) {
	uint32_t tmpPrevBlock	= 0;
	uint32_t tmpNextBlock	= 0;
	
	//Edit block to pop
	initrdFS_readBlock(fs, blockToPop);

	tmpPrevBlock = fs->blockEntry->footer->prevBlock;
	tmpNextBlock = fs->blockEntry->footer->nextBlock;

	fs->blockEntry->footer->prevBlock = 0;
	fs->blockEntry->footer->nextBlock = 0;

	initrdFS_writeBlock(fs, blockToPop);

	//If prev block exists
	if (tmpPrevBlock) {
		initrdFS_readBlock(fs, tmpPrevBlock);
		fs->blockEntry->footer->nextBlock = tmpNextBlock;
		initrdFS_writeBlock(fs, tmpPrevBlock);	
	}

	//If next block exists
	if (tmpNextBlock) {
		initrdFS_readBlock(fs, tmpNextBlock);
		fs->blockEntry->footer->prevBlock = tmpPrevBlock;
		initrdFS_writeBlock(fs, tmpNextBlock);	
	}
	
	return tmpNextBlock;
}

uint32_t initrdFS_getBlockSize(initrdFS_t* fs) {
	initrdFS_readBlock(fs, 0);

	initrdFS_Header_t* header = PTR2HEADER(fs->buffer);
	if (header->magic == FS_MAGIC)
		return header->blockSize;

	return 0;
}

uint32_t initrdFS_getInodesFirstBlock(initrdFS_t* fs) {
	initrdFS_readBlock(fs, 0);

	initrdFS_Header_t* header = PTR2HEADER(fs->buffer);
	if (header->magic == FS_MAGIC)
		return header->inodesFirstBlock;

	return 0;
}

uint32_t initrdFS_getInodesPerBlock(initrdFS_t* fs) {
	initrdFS_readBlock(fs, 0);

	initrdFS_Header_t* header = PTR2HEADER(fs->buffer);
	if (header->magic == FS_MAGIC) {
		return header->inodesPerBlock;
	}

	return 0;
}

uint32_t initrdFS_getFistFreeBlock(initrdFS_t* fs) {
	initrdFS_readBlock(fs, 0);

	initrdFS_Header_t* header = PTR2HEADER(fs->buffer);
	if (header->magic == FS_MAGIC)
		return header->firstFreeBlock;

	return 0;
}

void initrdFS_setFistFreeBlock(initrdFS_t* fs, uint32_t newFirstFreeBlock) {
	initrdFS_readBlock(fs, 0);

	initrdFS_Header_t* header = PTR2HEADER(fs->buffer);
	if (header->magic == FS_MAGIC) {
		header->firstFreeBlock = newFirstFreeBlock;
		initrdFS_writeBlock(fs, 0);
	}

}

uint32_t initrdFS_node2block(initrdFS_t* fs, uint32_t node) {
	uint32_t inodesFirstBlock = initrdFS_getInodesFirstBlock(fs);
	uint32_t inodesPerBlock = initrdFS_getInodesPerBlock(fs);

	return inodesFirstBlock + (node / inodesPerBlock);
}

uint32_t initrdFS_getFreeBlock(initrdFS_t* fs) {
	uint32_t freeBlock = initrdFS_getFistFreeBlock(fs);
	if (freeBlock)
		initrdFS_setFistFreeBlock(fs, initrdFS_popBlock(fs, freeBlock));

	return freeBlock;
}

void initrdFS_initBlock(initrdFS_t* fs, uint32_t block, uint32_t inodeNumber, uint8_t flags, uint32_t prevBlock, uint32_t nextBlock) {
	initrdFS_readBlock(fs, block);

	memset(fs->blockEntry->dataPtr, 0, fs->blockEntry->dataSize);
	
	fs->blockEntry->header->magic		= FS_MAGIC;
	fs->blockEntry->header->INodeNumber = inodeNumber;
	fs->blockEntry->header->flags		= flags;

	fs->blockEntry->footer->magic		= FS_MAGIC;
	fs->blockEntry->footer->prevBlock	= prevBlock;
	fs->blockEntry->footer->nextBlock	= nextBlock;
	 
	initrdFS_writeBlock(fs, block);
}

void initrdFS_initBlocks(initrdFS_t* fs, uint32_t startBlock, uint32_t endBlock) {
	uint32_t inodesFirstBlock = initrdFS_getInodesFirstBlock(fs);
	for (uint32_t i = startBlock; i < endBlock - 1; i++) {
		initrdFS_initBlock(fs, i, i == inodesFirstBlock ? FS_FLAG_BUSY : 0, 0, i ? i - 1 : 0, i + 1);
	}
	initrdFS_initBlock(fs, endBlock, 0, 0, endBlock - 1, 0);
	initrdFS_popBlock(fs, inodesFirstBlock);
}

void initrdFS_initHeader(initrdFS_t* fs, uint32_t blockSize, uint32_t blockCount, uint32_t inodesFirstBlock) {
	initrdFS_readBlock(fs, 0);
	
	initrdFS_Header_t* header = PTR2HEADER(fs->buffer);
	header->magic				= FS_MAGIC;
	header->blockSize			= fs->device->blockSize;
	header->blockCount			= fs->device->blockCount;
	header->inodesPerBlock		= alignValue(BLOCKDATASIZE(fs->device->blockSize), sizeof(initrdFS_INode_t)) / sizeof(initrdFS_INode_t);
	header->inodesFirstBlock	= inodesFirstBlock;
	header->firstFreeBlock		= inodesFirstBlock + 1;
	header->uuid				= 0; //TODO where is unique id? mmm?

	initrdFS_writeBlock(fs, 0);
}

uint32_t initrdFS_expandBlock(initrdFS_t* fs, uint32_t block) {
	uint32_t freeBlock =  initrdFS_getFreeBlock(fs);
	if (freeBlock)
		initrdFS_pushBlock(fs, block, freeBlock);

	return freeBlock;
}

uint32_t initrdFS_findFreeInode(initrdFS_t* fs) {
	uint32_t currentBlock = initrdFS_getInodesFirstBlock(fs);

	while (1) {
		initrdFS_readBlock(fs, currentBlock);
		uint32_t inodesCount = initrdFS_getInodesPerBlock(fs);

		for (uint32_t i = 0; i < inodesCount; i++) {
			initrdFS_INode_t* inode = (initrdFS_INode_t*)(fs->blockEntry->dataPtr + (i * sizeof(initrdFS_INode_t)));
			if (!(inode->flags & FS_FLAG_BUSY))
				return inode->number;
		}

		if (!fs->blockEntry->footer->nextBlock)
			return 0;

		currentBlock = fs->blockEntry->footer->nextBlock;
	}
}

void initrdFS_initNode(initrdFS_t* fs, uint32_t nodeNumber, uint8_t flags, uint32_t dataBlock) {
	uint32_t needBlock = initrdFS_node2block(fs, nodeNumber) - 1;
	uint32_t currentBlock = initrdFS_getInodesFirstBlock(fs);

	while (1) {
		initrdFS_readBlock(fs, currentBlock);

		if (!needBlock--) {
			initrdFS_INode_t* inode = PTR2INODEN(fs->blockEntry->dataPtr, nodeNumber);
			
			inode->number		= nodeNumber;
			inode->flags		= flags;//FS_FLAG_DIR | FS_FLAG_BUSY;
			inode->dataBlock	= dataBlock;

			initrdFS_writeBlock(fs, currentBlock);
			break;
		}

		if (!fs->blockEntry->footer->nextBlock)
			break; //TODO extend inode pool here

		currentBlock = fs->blockEntry->footer->nextBlock;
	}
}

uint8_t initrdFS_findFS(initrdFS_t* fs) {
	initrdFS_readBlock(fs, 0);

	if (PTR2HEADER(fs->buffer)->magic == FS_MAGIC)
		return 1;

	return 0;
}

uint32_t initrdFS_readInodeData(initrdFS_t* fs, uint32_t node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	uint32_t readed = 0;
	uint32_t block = initrdFS_node2block(fs, node);

	initrdFS_readBlock(fs, block);
	uint32_t dataBlock = PTR2INODEN(fs->blockEntry->dataPtr, node)->dataBlock;
	uint32_t startBlock = offset / fs->blockEntry->dataSize;
	uint32_t offsetInStartBlock = offset % fs->blockEntry->dataSize;

	while (1) {
		initrdFS_readBlock(fs, dataBlock);

		if (startBlock)
			startBlock--;
		else {
			uint32_t toRead = readed + fs->blockEntry->dataSize <= size ? fs->blockEntry->dataSize - offsetInStartBlock : size - readed;
			memcpy(buffer + readed, fs->blockEntry->dataPtr + offsetInStartBlock, toRead);
			readed += toRead;
			offsetInStartBlock = 0;
		}

		if (!fs->blockEntry->footer->nextBlock || readed >= size)
			break;

		dataBlock = fs->blockEntry->footer->nextBlock;
	}

	return readed;
}

uint32_t initrdFS_writeInodeData(initrdFS_t* fs, uint32_t node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	uint32_t writed = 0;
	uint32_t block = initrdFS_node2block(fs, node);

	initrdFS_readBlock(fs, block);
	uint32_t dataBlock = PTR2INODEN(fs->blockEntry->dataPtr, node)->dataBlock;
	uint32_t startBlock = offset / fs->blockEntry->dataSize;
	uint32_t offsetInStartBlock = offset % fs->blockEntry->dataSize;

	while (1) {
		initrdFS_readBlock(fs, dataBlock);

		if (startBlock)
			startBlock--;
		else {
			uint32_t toWrite = writed + fs->blockEntry->dataSize <= size ? fs->blockEntry->dataSize - offsetInStartBlock : size - writed;
			memcpy(fs->blockEntry->dataPtr + offsetInStartBlock, buffer + writed, toWrite);
			writed += toWrite;
			initrdFS_writeBlock(fs, dataBlock);
			offsetInStartBlock = 0;
		}

		if (writed >= size)
			break;

		if (!fs->blockEntry->footer->nextBlock) {
			uint32_t newBlock = initrdFS_expandBlock(fs, dataBlock);
			if (!newBlock)
				break;

			dataBlock = newBlock;
		} else {
			dataBlock = fs->blockEntry->footer->nextBlock;
		}
	}

	return writed;
}

/*
void initrdFS_makeDir(initrdFS_t* fs, uint32_t rootNode, char* name) {
	uint32_t freeInode = initrdFS_findFreeInode(fs);
	if (!freeInode)
		return;

	uint32_t rootNodeBlock = initrdFS_node2block(fs, rootNode);

	initrdFS_Block_t* blockEntry = (initrdFS_Block_t*)kmalloc(sizeof(initrdFS_Block_t));
	
	initrdFS_readBlock(fs, rootNodeBlock);
	initrdFS_buf2block(fs, blockEntry);
	
	if (!(READ_FLAGS(PTR2INODEN(fs->blockEntry->dataPtr, rootNode)->flags, FS_FLAG_DIR | FS_FLAG_BUSY)))
		return;

	uint32_t dataBlock = PTR2INODEN(fs->blockEntry->dataPtr, rootNode)->dataBlock;

	initrdFS_readBlock(fs, dataBlock);
	initrdFS_buf2block(fs, blockEntry);

	uint32_t dirEntriesCount = initrdFS_getDirEntriesPerBlock(fs);

	for (uint32_t i = 0; i < dirEntriesCount; i++) {
		initrdFS_DirEntry_t* dirEntry = (initrdFS_DirEntry_t*)(fs->blockEntry->dataPtr + (i * sizeof(initrdFS_DirEntry_t)));

		if (!dirEntry->INodeNumber) {
			strcpy(dirEntry->name, name);
			dirEntry->INodeNumber = freeInode;
			initrdFS_writeBlock(fs, dataBlock);

			uint32_t block = initrdFS_getFreeBlock(fs);
			initrdFS_initNode(fs, freeInode, FS_FLAG_DIR | FS_FLAG_BUSY, block);

			break;
		}
	}

	kfree(blockEntry);
}
*/

void initrdFS_makeFS(initrdFS_t* fs) {
	initrdFS_initHeader(fs, 512, 1024, 1);
	initrdFS_initBlocks(fs, 1, fs->device->blockCount);
	uint32_t block = initrdFS_getFreeBlock(fs);
	initrdFS_initNode(fs, 0, FS_FLAG_DIR | FS_FLAG_BUSY, block);
}
