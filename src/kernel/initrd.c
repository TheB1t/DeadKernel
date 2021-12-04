#include "initrd.h"

//INITRDHeader_t INITRDNode_t INITRDDirent_t INITRD_DIRECTORY INITRD_FILE

FSNode_t* initrdRoot = 0;

RDHeader_t* initrdHeader = 0;
RDNode_t* initrdNodes = 0;

static uint32_t RDRead(FSNode_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	RDNode_t readedNode = initrdNodes[node->inode];
	if (offset > readedNode.length)
		return 0;

	if (offset + size > readedNode.length)
		size = readedNode.length - offset;
		
	memcpy(buffer, (uint8_t*)(readedNode.start + offset), size);
	return size;
}

static Dirent_t* RDReadDir(FSNode_t* node, uint32_t index) {
	if (!(node->flags & FS_DIRECTORY))
		return 0;

	Dirent_t retDirent;
	Dirent_t* dirents = (Dirent_t*)node->start;
	uint32_t  dirents_size = node->length / sizeof(Dirent_t);

	if (index >= dirents_size)
		return 0;	

	memcpy(&retDirent, &dirents[index], sizeof(Dirent_t));
	return &retDirent;
}

static FSNode_t* RDFindDir(FSNode_t* node, char* name) {
	if (!(node->flags & FS_DIRECTORY))
		return 0;

	FSNode_t retNode;
	Dirent_t* dirent = (Dirent_t*)1;

	while (dirent) {
		dirent = RDReadDir(node, 0);
		if (strcmp(dirent->name, name)) {
			retNode.mask		= 0;
			retNode.uid			= 0;
			retNode.gid			= 0;
			retNode.inode		= initrdNodes[dirent->inode].inode;
			retNode.length		= initrdNodes[dirent->inode].length;
			retNode.start		= initrdNodes[dirent->inode].start;
			retNode.flags		= initrdNodes[dirent->inode].flags;
			retNode.read		= &RDRead;
			retNode.write		= 0;
			retNode.open		= 0;
			retNode.close		= 0;
			retNode.readDir		= &RDReadDir;
			retNode.findDir		= &RDFindDir;
			retNode.ptr			= 0;	
			return &retNode;
		}
	}
	
	return 0;
}

FSNode_t* initInitrd(uint32_t base) {
	initrdHeader = (RDHeader_t*)base;

	if (initrdHeader->magic != RD_MAGIC)
		PANIC("Initrd not found");

	initrdNodes = (RDNode_t*)(base + sizeof(RDHeader_t));
	
	initrdRoot = (FSNode_t*)kmalloc(sizeof(FSNode_t));
	initrdRoot->mask	= 0;
	initrdRoot->uid		= 0;
	initrdRoot->gid		= 0;
	initrdRoot->inode	= initrdNodes[0].inode;
	initrdRoot->length	= initrdNodes[0].length;
	initrdRoot->start	= initrdNodes[0].start;
	initrdRoot->flags	= initrdNodes[0].flags;
	initrdRoot->read	= &RDRead;
	initrdRoot->write	= 0;
	initrdRoot->open	= 0;
	initrdRoot->close	= 0;
	initrdRoot->readDir	= &RDReadDir;
	initrdRoot->findDir	= &RDFindDir;
	initrdRoot->ptr		= 0;

	return initrdRoot;
}
