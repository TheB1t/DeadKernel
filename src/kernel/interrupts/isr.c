#include "isr.h"

uint32_t interruptedEIP;
ISR_t interruptHandlers[256];

uint8_t* getISRName(uint32_t i) {
	switch (i) {
		case 0: return "Division by zero exception";
		case 1: return "Debug exception";
		case 2: return "Non maskable interrupt";
		case 3: return "Breakpoint exception";
		case 4: return "Into detected overflow";
		case 5: return "Out of bounds exception";
		case 6: return "Invalid opcode exception";
		case 7: return "No coprocessor exception";
		case 8: return "Double fault";
		case 9: return "Coprocessor segment overrun";
		case 10: return "Bad TSS";
		case 11: return "Segment not present";
		case 12: return "Stack fault";
		case 13: return "General protection fault";
		case 14: return "Page fault";
		case 15: return "Unknown interrupt exception";
		case 16: return "Coprocessor fault";
		case 17: return "Alignment check exception";
		case 18: return "Machine check exception";
		default: return "Unknown exception";
	}
}

void ISRHandler(registers_t regs) {
	interruptedEIP = regs.eip;
	if (interruptHandlers[regs.int_no] != 0) {
		interruptHandlers[regs.int_no](regs);
	} else {
		uint8_t* name = getISRName(regs.int_no);
		printf("Unhandled interrupt: %s\n", name);
		PANIC(name);
	}
}

void IRQHandler(registers_t regs) {
	interruptedEIP = regs.eip;
	if (regs.int_no >= 40)
		outb(0xA0, 0x20);

	outb(0x20, 0x20);

	if (interruptHandlers[regs.int_no] != 0)
		interruptHandlers[regs.int_no](regs);
}

void registerInterruptHandler(uint8_t n, ISR_t handler) {
	interruptHandlers[n] = handler;
}

void unregisterInterruptHandler(uint8_t n) {
	interruptHandlers[n] = NULL;
}
