#include <drivers/pci/pci.h>
#include <drivers/bios32/bios32.h>

static AddrIndirect PCIBIOSIndirect = { 0, 0 };
static uint8_t PCI_HWMech, PCI_majorVer, PCI_minorVer;

uint8_t PCIInit() {
	if (PCICheckSupport(&PCI_majorVer, &PCI_minorVer, &PCI_HWMech) == PCI_OP_SUCC) {
		LOG_INFO("[PCI] PCI BIOS found %02x ver %02x.%02x", PCI_HWMech, PCI_majorVer, PCI_minorVer);
		return PCI_OP_SUCC;
	} else {
		LOG_INFO("[PCI] BIOS32 instance not found!");
		return PCI_OP_FAIL;
	}
}

uint8_t PCICheckSupport(uint8_t* majorVer, uint8_t* minorVer, uint8_t* HWMech) {
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
		DISABLE_INTERRUPTS();

		asm volatile("		\
			lcall *(%%edi);	\
			jc 1f;			\
			xor %%ah, %%ah;	\
			1:				"
			: "=d" (signature), "=a" (eax), "=b" (ebx), "=c" (ecx)
			: "1" (PCIBIOS_PCI_BIOS_PRESENT), "D" (&PCIBIOSIndirect)
			: "memory"
		);

		ENABLE_INTERRUPTS();
		
		status		= (eax >> 8) & 0xFF;
		*HWMech		= eax & 0xFF;
		*majorVer	= (ebx >> 8) & 0xFF;
		*minorVer	= ebx & 0xFF;

		if (!status || signature == PCI_SIGNATURE)
			return PCI_OP_SUCC;
	}
	return PCI_OP_FAIL;
}

uint8_t PCIDirectRead(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t *value) {
    if (bus > 255 || dev > 31 || fn > 7 || reg > 255)
           return PCI_OP_FAIL;

	outl(0xCF8, PCI_CONF1_ADDRESS(bus, dev, fn, reg));
	switch (len) {
		case 1:	*value = (inl(0xCFC) >> ((reg & 3) * 8)) & 0x000000FF;	break;
		case 2:	*value = (inl(0xCFC) >> ((reg & 2) * 8)) & 0x0000FFFF;	break;
		case 4:	*value =  inl(0xCFC)					 & 0xFFFFFFFF;	break;
		default: return PCI_OP_FAIL;
    }

	return PCI_OP_SUCC;
}

uint8_t PCIDirectWrite(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t value) {
    if (bus > 255 || dev > 31 || fn > 7 || reg > 255)
           return PCI_OP_FAIL;

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
		default: return PCI_OP_FAIL;
    }
	outl(0xCF8, PCI_CONF1_ADDRESS(bus, dev, fn, reg));
	outl(0xCFC, data);
	
	return PCI_OP_SUCC;
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
					return PCI_OP_SUCC;
			}
		}
	}

    return PCI_OP_FAIL;
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
