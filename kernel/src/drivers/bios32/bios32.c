#include <drivers/bios32/bios32.h>

static AddrIndirect BIOS32Indirect = { 0, 0 };

extern BIOS32_t* find_bios32(void);

uint32_t BIOS32GetAddress() {
	return BIOS32Indirect.address;
}

uint8_t BIOS32Find() {
	BIOS32_t* ptr = find_bios32();

	if (!ptr)
		return BIOS32_FAIL;

	if (ptr->signature != BIOS32_SIGNATURE)
		return BIOS32_FAIL;

	if (ptr->entry >= 0x100000)
		return BIOS32_FAIL;

	if (ptr->revision != 0x00)
		return BIOS32_FAIL;

	if (ptr->length != 0x01)
		return BIOS32_FAIL;

	uint8_t checksum = 0;
	for (uint8_t i = 0; i < 16; i++)
		checksum += ((uint8_t*)ptr)[i];

	if (checksum)
		return BIOS32_FAIL;

	BIOS32Indirect.address = ptr->entry;
	asm volatile("mov %%cs, %0" : "=r" (BIOS32Indirect.segment));

	return BIOS32_SUCC; //Yeah, successful
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
	DISABLE_INTERRUPTS();

	asm volatile("		\
		lcall *(%%edi);	"
		:"=a" (return_code), "=b" (address), "=c" (length), "=d" (entry) 
		:"0" (service), "1" (0), "D" (&BIOS32Indirect)
	);

	ENABLE_INTERRUPTS();
	
	if (!return_code)
		return address + entry;

	return BIOS32_FAIL;
}