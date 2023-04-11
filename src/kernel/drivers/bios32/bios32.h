#pragma once

#include "common.h"

#define BIOS32_SIGNATURE				*(uint32_t*)"_32_"
#define PCI_SIGNATURE					*(uint32_t*)"PCI "
#define PCI_SERVICE						*(uint32_t*)"$PCI"

#define BIOS32_SUCC						0x01
#define BIOS32_FAIL						0x00

typedef struct {
	uint32_t	signature;
	uint32_t	entry;
	uint8_t		revision;
	uint8_t		length;
	uint8_t		checksum;
	uint8_t		reserved[5]; 
} BIOS32_t;

typedef struct {
	uint32_t	address;
	uint16_t	segment;
} AddrIndirect;

uint32_t	BIOS32GetAddress();
uint8_t		BIOS32Find();
uint32_t	BIOS32GetService(uint32_t service);