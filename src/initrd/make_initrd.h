
#include <dirent.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "loopDevice.h"

#define RD_MAGIC		0x52414D44
#define RD_INODE_POOL	16

#define RD_FILE			0x00000001
#define RD_DIRECTORY	0x00000002

#define RD_BUSY			0x00000004

typedef struct {
	uint32_t flags;
	uint32_t inode;
	uint32_t usedBlocks;
} RDNode_t;

typedef struct {
	uint8_t flags;
	char name[128];
	uint32_t inode;
} RDDirent_t;

typedef struct {
	uint32_t magic;
	uint32_t flags;
	uint32_t usedSpace;
	uint32_t reserved;
} RDBlockHeader_t;

typedef struct {
	uint32_t magic;
	uint32_t blockSize;
	uint32_t sizeInBlocks;
	uint32_t inodePoolSize;
} RDHeader_t;

