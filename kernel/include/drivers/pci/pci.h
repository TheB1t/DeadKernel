#pragma once

#include <utils/common.h>
#include <drivers/pci/pci_types.h>

uint8_t		PCIInit();
uint8_t 	PCICheckSupport(uint8_t* majorVer, uint8_t* minorVer, uint8_t* HWMech);
uint8_t		PCIDirectRead(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t *value);
uint8_t		PCIDirectWrite(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t value);
uint16_t	PCIGetVendorID(uint8_t bus, uint8_t dev, uint8_t fn);
uint16_t	PCIGetDeviceID(uint8_t bus, uint8_t dev, uint8_t fn);
uint32_t	PCIGetClassCode(uint8_t bus, uint8_t dev, uint8_t fn);
uint32_t	PCIDirectFindClass(uint32_t classCode, PCIDevice_t *pd);
uint32_t	PCIDirectScan(PCIDevice_t* devices);
