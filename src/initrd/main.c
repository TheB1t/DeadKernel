#include <stdio.h>
#include "blockDevice.h"


int main() {
	BlockDevice_t* dev = (BlockDevice_t*)malloc(sizeof(BlockDevice_t));
	BlockDevice_init(&dev, 512, 512);

	char data[] = "deadbeef";
	BlockDevice_writeBlock(dev, 16, 64, sizeof(data), (uint8_t*)data);

	char buffer[256];
	BlockDevice_readBlock(dev, 16, 64, sizeof(data), (uint8_t*)buffer);

	printf("%s\n", buffer);
	
	BlockDevice_deInit(&dev);
}
