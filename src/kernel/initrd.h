#pragma once

#include "common.h"
#include "kheap.h"
#include "fs.h"

#define RD_MAGIC		0x52414D44

#define RD_FILE			0x01
#define RD_DIRECTORY	0x02

typedef struct RDNode {
	uint32_t flags;
	uint32_t inode;
	uint32_t start;
	uint32_t length;
} RDNode_t;

typedef struct RDDirent {
	char name[128];
	uint32_t inode;
} RDDirent_t;

typedef struct RDHeader {
	uint32_t magic;
	uint32_t size;
} RDHeader_t;

FSNode_t* initInitrd(uint32_t base);
