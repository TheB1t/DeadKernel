#include <utils/common.h>
#include <interrupts/isr.h>
#include <multitasking/task.h>
#include <multitasking/systimer.h>
#include <utils/stackTrace.h>

uint32_t __interruptsDisable = 0;

void memPrint(uint8_t* mem, uint32_t size) {
	#define OUT_W 16
	
	serialprintf(COM1, "---- memory start at 0x%08x ----\n", mem);
	for (uint32_t i = 0; i < size / OUT_W; i++) {
		serialprintf(COM1, "%08x | ", mem + (OUT_W * i));
		for (uint32_t j = 0; j < OUT_W; j++) {
			serialprintf(COM1, "%02x ", mem[(OUT_W * i) + j]);
		}
		serialprintf(COM1, "| ");
		for (uint32_t j = 0; j < OUT_W; j++) {
			if (mem[(OUT_W * i) + j]  > 31)
				serialprintf(COM1, "%.1c", mem[(OUT_W * i) + j]);
			else
				serialprintf(COM1, ".");
		}
		serialprintf(COM1, "\n");
	}
	serialprintf(COM1, "---- memory end at 0x%08x ----\n", mem + size);
}

void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value) {
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void outl(uint16_t port, uint32_t value) {
	asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

void sleep(uint32_t ms) {
	uint32_t targetTicks = getSysTimerTicks() + ms;
	while (getSysTimerTicks() < targetTicks);
}

void kernel_warn(const char* message) {
	printf("[Kernel Warning] %s\n", message);
}

uint8_t panicCounter = 0;

void kernel_panic(const char* message) {
	if (panicCounter >= 1) {
		printf("Double panic!\n");
		for(;;);
	}
	
	panicCounter++;
	DISABLE_INTERRUPTS();
	printf("[Kernel Panic] %s at address 0x%08x\n", message, getInterruptedContext()->eip);
	stackTrace(8);
	for(;;);
}

void kernel_assert(const char* message, const char* file, uint32_t line) {
	DISABLE_INTERRUPTS();
	printf("[Assertion Failed] %s (%s:%d)", message, file, line);
	for(;;);
}

void kernel_halt() {
	DISABLE_INTERRUPTS();
	uint8_t good = 0x02;
	while (good & 0x02)
		good = inb(0x64);
	outb(0x64, 0xFE);
loop:
	asm volatile("hlt");
	goto loop;
}
