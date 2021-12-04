#include "systimer.h"

uint32_t tick = 0;

static void timerCallback(registers_t regs) {
	tick++;
	switchTask();
}

void initSysTimer(uint32_t freq) {
	registerInterruptHandler(IRQ0, &timerCallback);

	uint32_t divisor = 1193180 / freq;

	outb(0x43, 0x36);

	uint8_t	l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x40, l);
	outb(0x40, h);
}
