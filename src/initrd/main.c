#include <stdio.h>
#include "blockDevice.h"
#include "initrdFS.h"

//file for testing
int main() {
	BlockDevice_t* dev;
	BlockDevice_init(&dev, 512, 512);

	initrdFS_t* fs;
	initrdFS_init(&fs, dev);

	initrdFS_makeFS(fs);

	if (initrdFS_findFS(fs))
		printf("%s\n", "FOUND");

	//initrdFS_makeDir(fs, 0, "TEST");
	char data[4096 * 4];
	for (uint32_t i = 0; i < sizeof(data); i++)
		data[i] = rand() % 255;

	initrdFS_writeInodeData(fs, 0, 0, sizeof(data), (uint8_t*)data);

	char badchar = 0xDE;
	initrdFS_writeInodeData(fs, 0, (512 / 2) + 1024, 1, (uint8_t*)&badchar);

	char badchar2 = 0;
	initrdFS_readInodeData(fs, 0, (512 / 2) + 1024, 1, (uint8_t*)&badchar2);

	printf("%s\n", badchar == badchar2 ? "INJECTED" : "INJECT FAILED");

	char buffer[4096 * 4];
	initrdFS_readInodeData(fs, 2, 0, sizeof(data), (uint8_t*)buffer);

	uint8_t passed = 1;
	for (uint32_t i = 0; i < sizeof(data); i++) {
		if (data[i] != buffer[i]) {
			printf("%d\n", i);
			passed = 0;
			break;
		}
	}

	printf("%s\n", passed ? "PASSED" : "FAILED");

	initrdFS_deInit(&fs);
	BlockDevice_deInit(&dev);
}
