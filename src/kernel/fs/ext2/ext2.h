#pragma once

#include "../../common.h"

#define	EXT2_STATUS_OK					0x01
#define EXT2_STATUS_ERROR				0x02

#define	EXT2_ISERROR_IGNORE				0x01
#define	EXT2_ISERROR_REMOUNT			0x02
#define EXT2_ISERROR_PANIC				0x03

#define EXT2_OSID_LINUX					0x00
#define EXT2_OSID_GNU_HURD				0x01
#define EXT2_OSID_MASIX					0x02
#define EXT2_OSID_FREEBSD				0x03
#define EXT2_OSID_OTHER					0x04

#define EXT2_RFLAGS_COMPRESSION			0x0001
#define EXT2_RFLAGS_DIR_TYPE			0x0002
#define EXT2_RFLAGS_JOURNAL				0x0004
#define EXT2_RFLAGS_DJOURNAL			0x0008

#define EXT2_ROFLAGS_SPARSE				0x0001
#define EXT2_ROFLAGS_64SIZE				0x0002
#define EXT2_ROFLAGS_BTREE				0x0004

#define EXT2_INODE_TYPE_FIFO			0x1000
#define EXT2_INODE_TYPE_CHARDEV			0x2000
#define EXT2_INODE_TYPE_DIR				0x4000
#define EXT2_INODE_TYPE_BLOCKDEV		0x6000
#define EXT2_INODE_TYPE_REGFILE			0x8000
#define EXT2_INODE_TYPE_SYMFILE			0xA000
#define EXT2_INODE_TYPE_UNIXSOCK		0xC000

#define EXT2_INODE_PERMISSION_OX		0x001
#define EXT2_INODE_PERMISSION_OW		0x002
#define EXT2_INODE_PERMISSION_OR		0x004
#define EXT2_INODE_PERMISSION_GX		0x008
#define EXT2_INODE_PERMISSION_GW		0x010
#define EXT2_INODE_PERMISSION_GR		0x020
#define EXT2_INODE_PERMISSION_UX		0x040
#define EXT2_INODE_PERMISSION_UW		0x080
#define EXT2_INODE_PERMISSION_UR		0x100
#define EXT2_INODE_PERMISSION_SB		0x200
#define EXT2_INODE_PERMISSION_SGID		0x400
#define EXT2_INODE_PERMISSION_SUID		0x800

#define EXT2_INODE_FLAGS_SECURE_DEL		0x00000001
#define EXT2_INODE_FLAGS_CORY_SAVE		0x00000002
#define EXT2_INODE_FLAGS_FILE_COMPRESS	0x00000004
#define EXT2_INODE_FLAGS_SYNC_UPDATE	0x00000008
#define EXT2_INODE_FLAGS_IMMUTABLE		0x00000010
#define EXT2_INODE_FLAGS_APPEND_ONLY	0x00000020
#define EXT2_INODE_FLAGS_NOT_DUMP		0x00000040
#define EXT2_INODE_FLAGS_ACCESS_NOUPD	0x00000080
#define EXT2_INODE_FLAGS_HASH_INDEX_DIR	0x00010000
#define EXT2_INODE_FLAGS_AFS_DIR		0x00020000
#define EXT2_INODE_FLAGS_JOURNAL_FILE	0x00040000
	
#define EXT2_INODE2GBLOCKN(i, ipg)		(((i) - 1) / (ipg))
#define EXT2_INODE2GBLOCKO(i, ipg)		(((i) - 1) % (ipg))
#define EXT2_INODE2GBLOCKB(i, si, sb)	(((i) * (is)) / (sb))

typedef struct {
	uint32_t	inodes;
	uint32_t	blocks;
	uint32_t	superuserBlocks;
	uint32_t	unallocatedBlocks;
	uint32_t	unallocatedInodes;
	uint32_t	superBlock;
	uint32_t	blockSize;
	uint32_t	fragmentSize;
	uint32_t	groupBlocks;
	uint32_t	groupInodes;
	uint32_t	lastMountTimestamp;
	uint32_t	lastWriteTimestamp;
	uint16_t	mountsLastCheck;
	uint16_t	mountsBeforeCheck
	uint16_t	ext2sign;
	uint16_t	status;
	uint16_t	isErrorDetected;
	uint16_t	minorVersion;
	uint32_t	lastCheckTimestamp;
	uint32_t	forcedCheckInterval;
	uint32_t	osid;
	uint32_t	majorVersion;
	uint16_t	reservedBlocksUser;
	uint16_t	reservedBlocksGroup;
} EXT2SuperBlock_t;

typedef struct {
	uint32_t	firstNotReservedIndex;
	uint16_t	indexStructSize;
	uint16_t	superBlockGroup;
	uint32_t	optionalFeatureFlags;
	uint32_t	requiredFeatureFlags;
	uint32_t	readonlyFeatureFlags;
	uint8_t		fsUID[16];
	uint8_t		volumeName[16];
	uint8_t		lastMountedPath[64];
	uint32_t	compression;
	uint8_t		preallocateBlockFiles;
	uint8_t		preallocateBlockDirectories;
	uint16_t	unused0;
	uint8_t		jourbalID[16];
	uint32_t	journalInode;
	uint32_t	journalDevice;
	uint32_t	inodeListHead;
} EXT2SuperBlockExtended_t;

typedef struct {
	uint32_t	blockBitmap;
	uint32_t	inodeBitmap;
	uint32_t	inodeTable;
	uint16_t	unallocatedBlocks;
	uint16_t	unallocatedInodes;
	uint16_t	directories;
} EXT2BlockGroup_t;

typedef struct {
	uint16_t	typeAndPermissions;
	uint16_t	userID;
	uint32_t	lowerSize;
	uint32_t	lastAccessTimestamp;
	uint32_t	creationTimestamp;
	uint32_t	lastModificationTimestamp;
	uint32_t	deletionTimestamp;
	uint16_t	groupID;
	uint16_t	hardLinks;
	uint32_t	diskSectors;
	uint32_t	flags;
	uint32_t	osSpecificValue1;
	uint32_t	directBlockPointers[12];
	uint32_t	singlyIndirectBlockPointer;
	uint32_t	doublyIndirectBlockPointer;
	uint32_t	triplyInderectBlockPointer;
	uint32_t	generationNumber;
	uint32_t	reserver[2];
	uint32_t	fragmentBlockAddress;
	uint8_t		osSpecificValue2[12];
} EXT2Inode_t;


typedef struct {
	uint32_t	inode;
	uint16_t	size;
	uint8_t		nameLengthLower;
	uint8_t		nameLengthHigher;
} EXT2DirectoryEntry_t;
