#pragma once

#include "common.h"

#define BIOS32_SIGNATURE				*(uint32_t*)"_32_"
#define PCI_SIGNATURE					*(uint32_t*)"PCI "
#define PCI_SERVICE						*(uint32_t*)"$PCI"

#define PCIBIOS_PCI_BIOS_PRESENT		0xb101
#define PCIBIOS_FIND_PCI_DEVICE			0xb102
#define PCIBIOS_FIND_PCI_CLASS_CODE		0xb103
#define PCIBIOS_READ_CONFIG_BYTE		0xb108
#define PCIBIOS_READ_CONFIG_WORD		0xb109
#define PCIBIOS_READ_CONFIG_DWORD		0xb10a

#define SUCC							0x01
#define FAIL							0x00

#define PCI_CONF1_ADDRESS(bus, dev, fn, reg) ((uint32_t)0x80000000 | ((uint32_t)(bus) << 16) | ((uint32_t)(dev) << 11) | ((uint32_t)(fn) << 8) | ((uint32_t)(reg) & 0xFC))

#define PCI_SUCC						0x00
#define PCI_NOT_SUPPORTED				0x81
#define PCI_BAD_VENDOR					0x83
#define PCI_DEVICE_NOT_FOUND			0x86
#define PCI_BAD_REGISTER				0x87
#define PCI_SET_FAILED					0x88
#define PCI_BUFFER_TOO_SMALL			0x89

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

typedef struct {
	uint16_t	vendor, device;
	uint32_t	class;
	uint32_t	baseAddr;
	uint8_t		bus, dev, fn; 
} PCIDevice_t;

uint32_t	BIOS32GetAddress();
uint8_t		BIOS32Find();
uint32_t	BIOS32GetService(uint32_t service);
uint8_t		BIOS32CheckPCI(uint8_t* majorVer, uint8_t* minorVer, uint8_t* HWMech);

uint8_t		PCIDirectRead(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t *value);
uint8_t		PCIDirectWrite(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t value);
uint16_t	PCIGetVendorID(uint8_t bus, uint8_t dev, uint8_t fn);
uint16_t	PCIGetDeviceID(uint8_t bus, uint8_t dev, uint8_t fn);
uint32_t	PCIGetClassCode(uint8_t bus, uint8_t dev, uint8_t fn);
uint32_t	PCIDirectFindClass(uint32_t classCode, PCIDevice_t *pd);
uint32_t	PCIDirectScan(PCIDevice_t* devices);
