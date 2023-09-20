#include "systimer.h"

#define PIT_ONE_MS_DIVISOR		1193		// really 837us
#define INTERVAL(var, val)					\
	if ((tick - var) >= (val)) {			\
		var = tick;

uint32_t uptime				= 0;
uint32_t lastTaskTick		= 0;
uint32_t tick				= 0;

static void timerCallback(CPURegisters_t* regs) {
	tick++;
	uptime++;

	switchTask(regs);
}

void initSysTimer() {
	registerInterruptHandler(IRQ0, &timerCallback);

	uint32_t divisor = PIT_ONE_MS_DIVISOR;

	outb(0x43, 0x36);

	uint8_t	l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x40, l);
	outb(0x40, h);
}

uint32_t getUptime() {
	return uptime;
}

uint32_t getSysTimerTicks() {
	return tick;
}

void resetSysTimerTicks() {
	tick = 0;
}