#include "pci.h"

static AddrIndirect BIOS32Indirect = { 0, 0 };
static AddrIndirect PCIBIOSIndirect = { 0, 0 };
extern BIOS32_t* find_bios32(void);

uint32_t BIOS32GetAddress() {
	return BIOS32Indirect.address;
}

uint8_t BIOS32Find() {
	BIOS32_t* ptr = find_bios32();

	if (!ptr)
		return FAIL;

	if (ptr->signature != BIOS32_SIGNATURE)
		return FAIL;

	if (ptr->entry >= 0x100000)
		return FAIL;

	if (ptr->revision != 0x00)
		return FAIL;

	if (ptr->length != 0x01)
		return FAIL;

	uint8_t checksum = 0;
	for (uint8_t i = 0; i < 16; i++)
		checksum += ((uint8_t*)ptr)[i];

	if (checksum)
		return FAIL;

	BIOS32Indirect.address = ptr->entry;
	asm volatile("mov %%cs, %0" : "=r" (BIOS32Indirect.segment));

	return SUCC; //Yeah, successful
}

uint32_t BIOS32GetService(uint32_t service) {
    uint8_t return_code;
    uint32_t address, length, entry;
/*
	Input:
	EAX - service identificator
	EBX - function selector (always 0)
	EDI - BIOS32 entru point

	Output:
	AL - return code: 0 - service exists, 0x80 - service not supported
	EBX - physical service address
	ECX - service segment length
	EDX - service entry point
*/
	asm volatile("		\
		cli;			\
		lcall *(%%edi);	\
		sti				"
		:"=a" (return_code), "=b" (address), "=c" (length), "=d" (entry) 
		:"0" (service), "1" (0), "D" (&BIOS32Indirect)
	);

	if (!return_code)
		return address + entry;

	return FAIL;
}

uint8_t BIOS32CheckPCI(uint8_t* majorVer, uint8_t* minorVer, uint8_t* HWMech) {
	uint32_t signature, eax, ebx, ecx, PCIBIOSEntry;
	uint8_t status;

	if ((PCIBIOSEntry = BIOS32GetService(PCI_SERVICE))) {
		PCIBIOSIndirect.address = PCIBIOSEntry;
		asm volatile("mov %%cs, %0" : "=r" (PCIBIOSIndirect.segment));
/*	
		Input:
		EAX - service fuction number
		EDI - sevice entry point

		Output:
		EDX - service signature
		AH - service exists flag (0 – exists, otherwise - not exists)
		AL - Hardware Mechanism support
		BH - Interface major version number
		BL - Interface minor version number
		CL - Last PSI bus number
*/
		asm volatile("		\
			cli;			\
			lcall *(%%edi);	\
			jc 1f;			\
			xor %%ah, %%ah;	\
			1:				\
			sti				"
			: "=d" (signature), "=a" (eax), "=b" (ebx), "=c" (ecx)
			: "1" (PCIBIOS_PCI_BIOS_PRESENT), "D" (&PCIBIOSIndirect)
			: "memory"
		);

		status		= (eax >> 8) & 0xFF;
		*HWMech		= eax & 0xFF;
		*majorVer	= (ebx >> 8) & 0xFF;
		*minorVer	= ebx & 0xFF;

		if (!status || signature == PCI_SIGNATURE)
			return SUCC;
	}
	return FAIL;
}

uint8_t PCIDirectRead(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t *value) {
    if (bus > 255 || dev > 31 || fn > 7 || reg > 255)
           return FAIL;

	outl(0xCF8, PCI_CONF1_ADDRESS(bus, dev, fn, reg));
	switch (len) {
		case 1:	*value = (inl(0xCFC) >> ((reg & 3) * 8)) & 0x000000FF;	break;
		case 2:	*value = (inl(0xCFC) >> ((reg & 2) * 8)) & 0x0000FFFF;	break;
		case 4:	*value =  inl(0xCFC)					 & 0xFFFFFFFF;	break;
		default: return FAIL;
    }

	return SUCC;
}

uint8_t PCIDirectWrite(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t value) {
    if (bus > 255 || dev > 31 || fn > 7 || reg > 255)
           return FAIL;

	outl(0xCF8, PCI_CONF1_ADDRESS(bus, dev, fn, reg));
	uint32_t data = inl(0xCFC);
	
	switch (len) {
		case 1:
			data &= ~0x000000FF;
			data |= value & 0x000000FF;
			break;
		case 2:
			data &= ~0x0000FFFF;
			data |= value & 0x0000FFFF;
			break;
		case 4:
			data &= ~0xFFFFFFFF;
			data |= value & 0xFFFFFFFF;
			break;
		default: return FAIL;
    }
	outl(0xCF8, PCI_CONF1_ADDRESS(bus, dev, fn, reg));
	outl(0xCFC, data);
	
	return SUCC;
}

uint16_t PCIGetVendorID(uint8_t bus, uint8_t dev, uint8_t fn) {
	uint16_t vendorID = 0;
	PCIDirectRead(bus, dev, fn, 0x00, 2, (uint32_t*)&vendorID);
	return vendorID;
}

uint16_t PCIGetDeviceID(uint8_t bus, uint8_t dev, uint8_t fn) {
	uint16_t deviceID = 0;
	PCIDirectRead(bus, dev, fn, 0x02, 2, (uint32_t*)&deviceID);
	return deviceID;
}

uint32_t PCIGetClassCode(uint8_t bus, uint8_t dev, uint8_t fn) {
	uint32_t classCode = 0;
	PCIDirectRead(bus, dev, fn, 0x08, 4, &classCode);
	return classCode >> 8;
}

uint32_t PCIDirectFindClass(uint32_t classCode, PCIDevice_t* pd) {
    memset(pd, 0, sizeof(PCIDevice_t));

    for (uint8_t bus = 0; bus < 256; bus++) {
		for (uint8_t dev = 0; dev < 32; dev++) {
			for (uint8_t fn = 0; fn < 8; fn++) {
				if (PCIGetClassCode(bus, dev, fn) == classCode)	
					return SUCC;
			}
		}
	}

    return FAIL;
}

uint32_t PCIDirectScan(PCIDevice_t* devices) {
	uint32_t counter = 0;
	PCIDevice_t* ptr = devices;
	for (uint32_t bus = 0; bus < 256; bus++) {
		for (uint32_t dev = 0; dev < 32; dev++) {
			for (uint32_t fn = 0; fn < 8; fn++) {
				uint16_t vendorID = PCIGetVendorID(bus, dev, fn);
				if (vendorID != 0xFFFF) {
					ptr->vendor	= vendorID;
					ptr->device	= PCIGetDeviceID(bus, dev, fn);
					ptr->class	= PCIGetClassCode(bus, dev, fn);
					ptr->bus	= bus;
					ptr->dev	= dev;
					ptr->fn		= fn;
					ptr++;
					counter++;
				}
			}
		}
	}
	return counter;
}
