#pragma once

#include <stdlib.h>

#define PCIBIOS_PCI_BIOS_PRESENT		0xb101
#define PCIBIOS_FIND_PCI_DEVICE			0xb102
#define PCIBIOS_FIND_PCI_CLASS_CODE		0xb103
#define PCIBIOS_READ_CONFIG_BYTE		0xb108
#define PCIBIOS_READ_CONFIG_WORD		0xb109
#define PCIBIOS_READ_CONFIG_DWORD		0xb10a

#define PCI_OP_SUCC						0x01
#define PCI_OP_FAIL						0x00

#define PCI_CONF1_ADDRESS(bus, dev, fn, reg) ((uint32_t)0x80000000 | ((uint32_t)(bus) << 16) | ((uint32_t)(dev) << 11) | ((uint32_t)(fn) << 8) | ((uint32_t)(reg) & 0xFC))

#define PCI_SUCC						0x00
#define PCI_NOT_SUPPORTED				0x81
#define PCI_BAD_VENDOR					0x83
#define PCI_DEVICE_NOT_FOUND			0x86
#define PCI_BAD_REGISTER				0x87
#define PCI_SET_FAILED					0x88
#define PCI_BUFFER_TOO_SMALL			0x89

typedef struct {
	uint16_t	vendor, device;
	uint32_t	class;
	uint32_t	baseAddr;
	uint8_t		bus, dev, fn; 
} PCIDevice_t;