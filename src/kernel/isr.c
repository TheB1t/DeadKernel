#include "isr.h"

ISR_t interruptHandlers[256];

void ISRHandler(registers_t regs) {
	if (interruptHandlers[regs.int_no] != 0) {
		ISR_t handler = interruptHandlers[regs.int_no];
		handler(regs);
	} else {
		printf("Unhandled Interrupt: 0x%x\n", regs.int_no);
		//PANIC("Unhandled Interrupt");
	}
}

void IRQHandler(registers_t regs) {
	if (regs.int_no >= 40) {
		outb(0xA0, 0x20);
	}

	outb(0x20, 0x20);

	if (interruptHandlers[regs.int_no] != 0) {
		ISR_t handler = interruptHandlers[regs.int_no];
		handler(regs);
	}
}

void registerInterruptHandler(uint8_t n, ISR_t handler) {
	interruptHandlers[n] = handler;
}
