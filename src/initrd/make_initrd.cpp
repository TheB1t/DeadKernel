#include "make_initrd.h"

/*
void recursiveDirParser(std::string path, uint32_t inode) {
	DIR* d = opendir(path);
	if (d) {
		struct dirent* dir;
		while ((dir = readdir(d)) != NULL) {
			std::string name(dir->d_name);
			uint32_t type = dir->d_type;
			
			if (name == "." && name == "..")
				continue;

			
			
		}
		closedir(d);
	}
}
*/

BlockDevice*	mem;

void makeFS(BlockDevice* device) {
	uint8_t* buffer = new uint8_t[device->getBlockSize()];
	uint32_t headerBlock = 0;
	uint32_t inodesStartBlock = 1;

	//Init blocks
	for (uint32_t i = 0; i < device->getSizeInBlocks(); i++) {
		memset(buffer, 0, device->getBlockSize());
		RDBlockHeader_t* bheader	= (RDBlockHeader_t*)buffer;
		bheader->magic				= RD_MAGIC;
		bheader->flags				= 0;
		bheader->usedSpace			= 0;
		device->writeBlock(i, buffer);
	}

	//Init header
	memset(buffer, 0, device->getBlockSize());
	RDHeader_t* header			= (RDHeader_t*)buffer;
	header->magic				= RD_MAGIC;
	header->blockSize			= device->getBlockSize();
	header->sizeInBlocks		= device->getSizeInBlocks();
	header->inodePoolSize		= RD_INODE_POOL;

	device->writeBlock(0, buffer);

	//Init inodes
	for (uint32_t i = 0; i < header->inodePoolSize; i++) {
		device->readBlock(i + inodesStartBlock, buffer);

		RDBlockHeader_t* bheader	= (RDBlockHeader_t*)buffer;
		bheader->flags				|= RD_BUSY;
		
		RDNode_t* node		= (RDNode_t*)(buffer + sizeof(RDBlockHeader_t));
		node->flags			= 0;
		node->inode			= i;
		node->usedBlocks	= 0;		

		device->writeBlock(i + inodesStartBlock, buffer);
	}

	//Create root inode
	device->readBlock(1, buffer);
	
	RDNode_t* rootNode = (RDNode_t*)buffer;

	rootNode->flags = RD_DIRECTORY | RD_BUSY;
	rootNode->usedBlocks = 1;

	uint32_t* blocks = (uint32_t*)(buffer + sizeof(RDNode_t));
	blocks[0] = inodesStartBlock;

	device->writeBlock(1, buffer);

	//Set initial block to BUSY state for root inode
	device->readBlock(inodeStartBlock, buffer);
	
	RDBlockHeader_t* bheader	= (RDBlockHeader_t*)buffer;
	bheader->flags				= RD_BUSY;

	device->writeBlock(inodeStartBlock, buffer);

	//Free resources
	free(buffer);
}

uint32_t findFreeNode(BlockDevice* device) {
	uint32_t inode = 0;
	uint8_t* buffer = new uint8_t[device->getBlockSize()];
	
	for (uint32_t i = 0; i < header->inodePoolSize; i++) {
		device->readBlock(i + 1, buffer);

		RDNode_t* node = (RDNode_t*)(buffer + sizeof(RDBlockHeader_t));	
		
		if (node->flags & RD_BUSY)
			continue;

		inode = node->inode;
		break;
	}
	
	free(buffer);
	return inode;
}

uint32_t findFreeDirent(BlockDevice* device, uint32_t inode) {
	uint32_t dirent = 0;
	uint8_t* buffer = new uint8_t[device->getBlockSize()];
	uint8_t* bbuffer = new uint8_t[device->getBlockSize()];
	device->readBlock(1 + inode, buffer);

	RDNode_t* node = (RDNode_t*)(buffer + sizeof(RDBlockHeader_t));	
	
	if (!(node->flags & RD_DIRECTORY && node->flags & RD_BUSY))
		return 0;

	uint32_t* blocks = (uint32_t*)(buffer + sizeof(RDNode_t));
	for (uint32_t i = 0; i < node->usedBlocks; i++) {
		device->readBlock(blocks[i], bbuffer);

		for (uint32_t j = 0; j < ((device->getBlockSize() - sizeof(RDBlockHeader_t)) / sizeof(RDDirent_t)); j++) {
			RDDirent_t* dirent = (RDDirent_t*)(buffer + sizeof(RDBlockHeader_t) + j);
			if (dirent->flags & RD_BUSY)
				continue;

			dirent = j;
			break;
		}

		if (dirent != 0) 
			break;
	}

	free(buffer);
	free(bbuffer);
	return dirent;
}

uint32_t RDRead(BlockDevice* fs, uint32_t inode, uint32_t offset, uint32_t size, uint8_t* data) {
	uint8_t* buffer = new uint8_t[device->getBlockSize()];
	device->readBlock(1 + inode, buffer);

	RDNode_t* node = (RDNode_t*)(buffer + sizeof(RDBlockHeader_t));
	uint32_t length = fs->getBlockSize() * node->usedBlocks;
	if (offset > length)
		return 0;

	if (offset + size > length)
		size = length - offset;

	
	memcpy(data, (uint8_t*)(fs->mem + node->start + offset), size);
	return size;
}

uint32_t RDWrite(FS_t* fs, RDNode_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	memcpy((uint8_t*)(fs->mem + node->start + offset), size);
	
}

RDNode_t* addDir(FS_t* fs, RDNode_t* node, char* dirname) {
	RDNode_t* freeNode = findFreeNode(fs);
	if (!freeNode)
		return 0;

	RDDirent_t* freeDirent = findFreeDirent(fs, node);
	if (!freeDirent)
		return 0;
		
	freeNode->flags = RD_DIRECTORY | RD_BUSY;
	freeNode->start	= fs->freemem;
	freeNode->length = sizeof(RDDirent_t) * RD_DIRENT_POOL;
	fs->freemem += freeNode->length;

	memset(freeDirent->name, 0, sizeof(freeDirent->name));
	memcpy(freeDirent->name, dirname, strlen(dirname));
	freeDirent->flags = RD_BUSY;
	freeDirent->inode = freeNode->inode;
	
	return freeNode;
}

RDNode_t* addFile(FS_t* fs, RDNode_t* node, char* filename, uint8_t* data, uint32_t size) {
	RDNode_t* freeNode = findFreeNode(fs);
	if (!freeNode)
		return 0;

	RDDirent_t* freeDirent = findFreeDirent(fs, node);
	if (!freeDirent)
		return 0;

	freeNode->flags = RD_FILE | RD_BUSY;
	freeNode->start	= fs->freemem;
	freeNode->length = size;
	memcpy((char*)(fs->mem + freeNode->start), data, size);
	
	fs->freemem += freeNode->length;

	memset(freeDirent->name, 0, sizeof(freeDirent->name));
	memcpy(freeDirent->name, filename, strlen(filename));
	freeDirent->flags = RD_BUSY;
	freeDirent->inode = freeNode->inode;
	
	return freeNode;	
}

void printFS(FS_t* fs, RDNode_t* node, char* dir, uint32_t level) {
	if (node->flags & RD_DIRECTORY | RD_BUSY) {
		RDDirent_t* dirents			= (RDDirent_t*)(fs->mem + node->start);
		uint32_t	dirents_size	= node->length / sizeof(RDDirent_t);

		for (uint32_t i = 0; i < dirents_size; i++) {
			if (!(dirents[i].flags & RD_BUSY))
				continue;

			RDNode_t* tempNode = &fs->nodes[dirents[i].inode];				
			if (tempNode->flags & RD_DIRECTORY && tempNode->flags & RD_BUSY) {
				for (uint32_t i = 0; i < level; i++) {
					std::cout << "|-";
				}
				std::cout << dirents[i].name << std::endl;
				printFS(fs, tempNode, (char*)(std::string(dir) + std::string(dirents[i].name) + "/").c_str(), level  + 1);
			}

			if (tempNode->flags & RD_FILE && tempNode->flags & RD_BUSY) {
				for (uint32_t i = 0; i < level; i++) {
					std::cout << "|-";
				}
				std::cout << dirents[i].name << " contains: ";
				char* content = (char*)malloc(tempNode->length + 1);
				memcpy(content, (char*)(fs->mem + tempNode->start), tempNode->length);
				content[tempNode->length] = '\0';
				std::cout << content << std::endl;
				free(content);				
			}
			
		}
	}	
}

void usage() {
	printf("Usage: ./make_initrd [dir]");
}

int main (char argc, char* argv[]) {
	if (argc < 2) {
		usage();
		return 0;
	}

	char* dir = argv[1];

	FS_t newfs;
	RDNode_t* rootNode = makeFS(&newfs, 64, 64 * 1024);
	printf("Total filesystem size %d bytes\n", newfs.memSize);

	RDNode_t* ndir = addDir(&newfs, rootNode, "etc");
	RDNode_t* ndir2 = addDir(&newfs, ndir, "apt");
	RDNode_t* ndir3 = addDir(&newfs, ndir2, "sources.list.d");
	
	addFile(&newfs, ndir, "rc.conf", (uint8_t*)"RC CONF HERE", strlen("RC CONF HERE") + 1);
	addFile(&newfs, ndir, "dhcpd.conf", (uint8_t*)"dhcpd CONF HERE", strlen("dhcpd CONF HERE") + 1);
	addFile(&newfs, ndir2, "sources.list", (uint8_t*)"SOURCES LIST HERE", strlen("SOURCES LIST HERE") + 1);
	
	printFS(&newfs, rootNode, "/", 0);
	//inodes[0] = { 0, 0,  };
	//recursive_dir_scanner(dir, cb);	
}
