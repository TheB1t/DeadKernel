#pragma once

#include "common.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?

struct FSNode;
struct Dirent;

typedef	uint32_t		(*readType_t)		(struct FSNode*, uint32_t, uint32_t, uint8_t*);
typedef	uint32_t		(*writeType_t)		(struct FSNode*, uint32_t, uint32_t, uint8_t*);
typedef	void			(*openType_t)		(struct FSNode*);
typedef void			(*closeType_t)		(struct FSNode*);
typedef struct Dirent*	(*readDirType_t)	(struct FSNode*, uint32_t);
typedef struct FSNode*	(*findDirType_t)	(struct FSNode*, char* name);

typedef struct FSNode {
	uint32_t mask;
	uint32_t uid;
	uint32_t gid;
	uint32_t flags;
	uint32_t inode;
	uint32_t start;
	uint32_t length;
	readType_t		read;
	writeType_t		write;
	openType_t		open;
	closeType_t 	close;
	readDirType_t	readDir;
	findDirType_t	findDir;
	struct FSNode* ptr;
} FSNode_t;

typedef struct Dirent {
	char name[128];
	uint32_t inode;
} Dirent_t;

uint32_t	readFS(FSNode_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t	writeFS(FSNode_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void		openFS(FSNode_t* node, uint8_t read, uint8_t write);
void		closeFS(FSNode_t* node);
Dirent_t*	readDirFS(FSNode_t* node, uint32_t index);
FSNode_t*	findDirFS(FSNode_t* node, char* name);
