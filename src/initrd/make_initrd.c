#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define FILE        0x01
#define DIRECTORY   0x02

typedef	unsigned int	uint32_t;
typedef			 int	int32_t;
typedef	unsigned short	uint16_t;
typedef			 short	int16_t;
typedef	unsigned char	uint8_t;
typedef 		 char	int8_t;

typedef struct {
	uint32_t	inode;
	uint8_t		flags;
	uint32_t	offset;
	uint32_t	length;
} initrdInode_t;

typedef struct {
	char		name[60];
	uint32_t	inode;
} initrdDirent_t;

typedef struct {
	initrdInode_t inodes[256];
	initrdDirent_t dirents[64];
} initrd_t;

uint32_t counter = 0;

void configureInode(initrdInode* inode, uint32_t num, uint8_t flags, uint32_t length) {
	*inode = { .inode = num, .flags = flags, .offset = 0, .length = length };
}

void configureInodeDir(initrdInode* inode, uint32_t num) {
	configureInode(inode, num, DIRECTORY, sizeof(initrdDirent) * 16);
}

void configureInodeFile(initrdInode* inode, uint32_t num, uint32_t size) {
	configureInode(inode, num, FILE, size);
}

void recursiveDirParser(char* path, uint32_t inode) {
	DIR* d = opendir(path);
	if (d) {
		struct dirent* dir;
		while ((dir = readdir(d)) != NULL) {
			char buffer[256];
			if (strcmp(".", dir->d_name) == 0 || strcmp("..", dir->d_name) == 0)
				continue;
	
			memset(buffer, 0, 256);
			strcat(buffer, path);
			if (strcmp("/", path) != 0 && strcmp("./", path) != 0 && strcmp("../", path) != 0)
				strcat(buffer, "/");

			strcat(buffer, dir->d_name);

			initrdInode* header = &inodes[inode];
			strcpy(header->name, name);
			if (!isDir) {
				FILE* stream = fopen(buffer, "r");
				fseek(stream, 0, SEEK_END);
				header->length = ftell(stream);
				header->offset = offset;
				fclose(stream);
					
				offset += header->length;
				printf("[%-064s] Packed on 0x%08X! File size %d\n", buffer, header->offset, header->length);
			} else {
				header->length = 0;
				header->offset = 0;		
				printf("[%-064s] Dir packed!\n", buffer);
			}
			header->magic = 0xBF;

			if (dir->d_type == 0x04)
				recursive_dir_scanner(buffer, callback);
		}
		closedir(d);
	}
}

void usage() {
	printf("Usage: ./make-initrd [dir]");
}

int main (char argc, char* argv[]) {
	if (argc < 2) {
		usage();
		return 0;
	}

	char* dir = argv[1];

	inodes[0] = { .inode = 0, .flags = 0, .offset = offset,  };
	recursive_dir_scanner(dir, cb);	
}
