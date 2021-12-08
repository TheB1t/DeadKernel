#include "pci.h"

static AddrIndirect BIOS32Indirect = { 0, 0 };
static AddrIndirect PCIBIOSIndirect = { 0, 0 };

uint32_t BIOS32GetAddress() {
	return BIOS32Indirect.address;
}

uint8_t BIOS32Find() {
	BIOS32_t* ptr = (BIOS32_t*)0xE0000;
	
	while (ptr->signature != BIOS32_SIGNATURE || (uint32_t)ptr < 0xEFFFF) ++ptr;

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

uint32_t PCIBIOSFindDevice(uint16_t vendor, uint16_t deviceID, uint16_t index, uint8_t* bus, uint8_t* dev, uint8_t* fn) {
    uint16_t bx, ret;

/*
	EAX – запрашиваемая функция сервиса, в данном случае 0xB102.
	ECX – код типа устройства.
	EDX – код фирмы-изготовителя устройства.
	ESI – индекс (порядковый номер) устройства заданного типа.
	В регистр EDI занесем адрес точки входа в сервис.
	В результате выполнения вызова в регистрах процессора будет находиться следующая информация:

	BH – номер шины, к которой подключено устройство;
	BL – номер устройства в старших пяти битах и номер функции в трёх младших;
	AH – код возврата (может принимать значения BAD_VEN-DOR_ID, DEVICE_NOT_FOUND и SUCCESFUL).
*/
    asm volatile("		\
    	cli;			\
    	lcall *(%%edi);	\
    	jc 1f;			\
    	xor %%ah, %%ah;	\
    	1:				\
    	sti				"
        : "=b" (bx), "=a" (ret)
        : "1" (PCIBIOS_FIND_PCI_DEVICE), "c" (deviceID), "d" (vendor), "S" ((int32_t)index), "D" (&PCIBIOSIndirect)
	);
	
    *bus	= (bx >> 8) & 0xFF;
    *dev	= (bx & 0xFF) >> 3;
    *fn		= bx & 0x3;
    
    return (uint32_t)(ret & 0xFF00) >> 8;
}

uint32_t PCIBIOSFindClass(uint32_t classCode, uint16_t index, PCIDevice_t* pd) {
	uint16_t bx, ret;
	
    asm volatile("		\
    	cli;			\
    	lcall *(%%edi);	\
    	jc 1f;			\
    	xor %%ah, %%ah;	\
    	1:				\
    	sti				"
        : "=b" (bx), "=a" (ret)
        : "1" (PCIBIOS_FIND_PCI_CLASS_CODE), "c" (classCode), "S" ((int32_t)index), "D" (&PCIBIOSIndirect)
	);

    pd->bus	= (bx >> 8) & 0xFF;
    pd->dev	= (bx & 0xFF) >> 3;
    pd->fn	= bx & 0x3;
    
    return (uint32_t)(ret & 0xFF00) >> 8;
}

uint32_t PCIBIOSRead(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t* value) {
	uint32_t bx, ret = 0;

    if (bus > 255 || dev > 31 || fn > 7 || reg > 255)
           return FAIL;

/*
	0xB108 – чтение байта;
	0xB109 – чтение слова;
	0xB10A – чтение двойного слова.

	EAX – код функции;
	BH – номер шины, к которой подключено устройство;
	BL – номер устройства в старших пяти битах и номер функции в трёх младших битах;
	DI – смещение в конфигурационном пространстве.
*/
	
	bx = ((bus << 8) | (dev << 3) | fn);

	uint32_t readFlags = 0;
	switch (len) {
		case 1: readFlags = PCIBIOS_READ_CONFIG_BYTE; break;
		case 2: readFlags = PCIBIOS_READ_CONFIG_WORD; break;
		case 4: readFlags = PCIBIOS_READ_CONFIG_DWORD; break;
	}

	if (!readFlags)
		return FAIL;
	
	asm volatile("		\
	   	cli;			\
	   	lcall *(%%edi);	\
	   	jc 1f;			\
	   	xor %%ah, %%ah;	\
	   	1:				\
	   	sti				"
		: "=c" (*value), "=a" (ret)
		: "1" (readFlags), "b" (bx), "D" ((long)reg), "S" (&PCIBIOSIndirect)
	);
			
    return (uint32_t)((ret & 0xFF00) >> 8);

}

uint32_t PCIDirectRead(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t reg, uint8_t len, uint32_t *value) {
    if (bus > 255 || dev > 31 || fn > 7 || reg > 255)
           return FAIL;

	outl(PCI_CONF1_ADDRESS(bus, dev, fn, reg), 0xCF8);
	switch (len) {
		case 1:
			*value = inb(0xCFC + (reg & 3));
			break;
		case 2:
			*value = inw(0xCFC + (reg & 2));
			break;
		case 4:
			*value = inl(0xCFC);
			break;
    }

	return SUCC;
}

uint32_t PCIDirectFindClass(uint32_t classCode, PCIDevice_t* pd) {
	uint8_t bus, dev, fn;
    uint32_t configDword, code;
    
    memset(pd, 0, sizeof(PCIDevice_t));

    for (uint8_t bus = 0; bus < 256; bus++) {
		for (uint8_t dev = 0; dev < 32; dev++) {
			for (uint8_t fn = 0; fn < 8; fn++) {
				PCIDirectRead(bus, dev, fn, 0x08, 4, &configDword);
				if ((configDword >> 8) == classCode)	
					return SUCC;
			}
		}
	}

    return FAIL;
}

char* PCIGetClassName(uint32_t classCode) {
	uint8_t baseClass = (classCode >> 16) & 0x000000FF;
	uint8_t subClass = (classCode >> 8) & 0x000000FF;
	switch (baseClass) {
		case 0x00:
			switch (subClass) {
				case 0x00: return "Non-VGA unclassified device";
				case 0x01: return "VGA compatible unclassified device";
				case 0x05: return "Image coprocessor";
				default: return "Unclassified device";
			}
		case 0x01:
			switch (subClass) {
				case 0x00: return "SCSI storage controller";
				case 0x01: return "IDE interface";
				case 0x02: return "Floppy disk controller";
				case 0x03: return "IPI bus controller";
				case 0x04: return "RAID bus controller";
				case 0x05: return "ATA controller";
				case 0x06: return "SATA controller";
				case 0x07: return "Serial Attached SCSI controller";
				case 0x08: return "Non-Volatile memory controller";
				case 0x80: return "Mass storage controller";
				default: return "Unclassified device";
			}
		case 0x02:
			switch (subClass) {
				case 0x00: return "Ethernet controller";
				case 0x01: return "Token ring network controller";
				case 0x02: return "FDDI network controller";
				case 0x03: return "ATM network controller";
				case 0x04: return "ISDN controller";
				case 0x05: return "WorldFip controller";
				case 0x06: return "PICMG controller";
				case 0x07: return "Infiniband controller";
				case 0x08: return "Fabric controller";
				case 0x80: return "Network controller";
				default: return "Unclassified device";
			}
		case 0x03:
			switch (subClass) {
				case 0x00: return "Multimedia video controller";
				case 0x01: return "Multimedia audio controller";
				case 0x02: return "Computer telephony device";
				case 0x03: return "Audio device";
				case 0x80: return "Multimedia controller";
				default: return "Unclassified device";
			}
		case 0x04:
			switch (subClass) {
				case 0x00: return "Multimedia video controller";
				case 0x01: return "Multimedia audio controller";
				case 0x02: return "Computer telephony device";
				case 0x03: return "Audio device";
				case 0x80: return "Multimedia controller";
				default: return "Unclassified device";
			}
		case 0x05:
			switch (subClass) {
				case 0x00: return "RAM memory";
				case 0x01: return "FLASH memory";
				case 0x80: return "Memory controller";
				default: return "Unclassified device";
			}
		case 0x06:
			switch (subClass) {
				case 0x00: return "Host bridge";
				case 0x01: return "ISA bridge";
				case 0x02: return "EISA bridge";
				case 0x03: return "MicroChannel bridge";
				case 0x04: return "PCI bridge";
				case 0x05: return "PCMCIA bridge";
				case 0x06: return "NuBus bridge";
				case 0x07: return "CardBus bridge";
				case 0x08: return "RACEway bridge";
				case 0x09: return "Semi-transpanent PCI-to-PCI bridge";
				case 0x0a: return "InfiniBand to PCI host bridge";
				case 0x80: return "Brige";
				default: return "Unclassified device";
			}
		case 0x07:
			switch (subClass) {
				case 0x00: return "Serial controller";
				case 0x01: return "Parallel controller";
				case 0x02: return "Multiport serial controller";
				case 0x03: return "Modem";
				case 0x04: return "GPIB controller";
				case 0x05: return "Smart Card controller";
				case 0x80: return "Communication controller";
				default: return "Unclassified device";
			}
		case 0x08:
			switch (subClass) {
				case 0x00: return "PIC";
				case 0x01: return "DMA controller";
				case 0x02: return "Timer";
				case 0x03: return "RTC";
				case 0x04: return "PCI Hot-plug controller";
				case 0x05: return "SD Host controller";
				case 0x06: return "IOMMU";
				case 0x80: return "System peripheral";
				case 0x90: return "Timing Card";
				default: return "Unclassified device";
			}
		case 0x09:
			switch (subClass) {
				case 0x00: return "Keyboard";
				case 0x01: return "Digitizer Pen";
				case 0x02: return "Mouse controller";
				case 0x03: return "Scanner controller";
				case 0x04: return "Gameport controller";
				case 0x80: return "Input device controller";
				default: return "Unclassified device";
			}
		case 0x0a:
			switch (subClass) {
				case 0x00: return "Generic Docking Station";
				case 0x80: return "Docking Station";
				default: return "Unclassified device";
			}
		case 0x0b:
			switch (subClass) {
				case 0x00: return "386";
				case 0x01: return "486";
				case 0x02: return "Pentium";
				case 0x10: return "Alpha";
				case 0x20: return "Power PC";
				case 0x30: return "MIPS";
				case 0x40: return "Co-processor";
				default: return "Unclassified device";
			}
		case 0x0c:
			switch (subClass) {
				case 0x00: return "Fire Wire (IEEE 1394)";
				case 0x01: return "ACCESS Bus";
				case 0x02: return "SSA";
				case 0x03: return "USB controller";
				case 0x04: return "Fibre Channel";
				case 0x05: return "SMBus";
				case 0x06: return "InfiniBand";
				case 0x07: return "IPMI Interface";
				case 0x08: return "SERCOS interface";
				case 0x09: return "CANBUS";
				default: return "Unclassified device";
			}
		case 0x0d:
			switch (subClass) {
				case 0x00: return "IRDA controller";
				case 0x01: return "Consumer IR controller";
				case 0x10: return "RF controller";
				case 0x11: return "Bluetooth";
				case 0x12: return "Boardband";
				case 0x20: return "802.1a controller";
				case 0x21: return "802.1b controller";
				case 0x80: return "Wireless controller";
				default: return "Unclassified device";
			}
		case 0x0e:
			switch (subClass) {
				case 0x00: return "I2O";
				default: return "Unclassified device";
			}
		case 0x0f:
			switch (subClass) {
				case 0x01: return "Sattelite TV controller";
				case 0x02: return "Sattelite audio communication controller";
				case 0x03: return "Sattelite voice communication controller";
				case 0x04: return "Sattelite data communication controller";
				default: return "Unclassified device";
			}
		case 0x10:
			switch (subClass) {
				case 0x00: return "Network and computing encryption device";
				case 0x10: return "Entertaiment encryption device";
				case 0x80: return "Encryption controller";
				default: return "Unclassified device";
			}
		case 0x11:
			switch (subClass) {
				case 0x00: return "DPIO module";
				case 0x01: return "Perfomance counters";
				case 0x10: return "Communication synchronizer";
				case 0x20: return "Signal processing managment";
				case 0x80: return "Signal processing controller";
				default: return "Unclassified device";
			}
		case 0x12:
			switch (subClass) {
				case 0x00: return "Processing accelerators";
				case 0x01: return "AI Interface Accelerator";
				default: return "Unclassified device";
			}
		case 0x13: return "Non-Essential Instrumentation";
		case 0x40: return "Coprocessor";
		case 0xFF: return "Unassigned class";
		default: return "Unclassified device";
	}
}
