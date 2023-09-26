#pragma once

#include <stdlib.h>
#include <io/screen.h>
#include <io/serial.h>
#include <utils/utils.h>

#define GDT_DESC_SEG(n, cpl)	(uint32_t)(((uint32_t)0x8 * (uint32_t)n) | (uint32_t)cpl)

#define HALT		kernel_halt()
#define WARN(msg)	kernel_warn(msg)
#define PANIC(msg)	kernel_panic(msg)
#define ASSERT(b)	((b) ? (void)0 : kernel_assert(#b, __FILE__, __LINE__))

#if defined(DEBUG_INTERRUPT_TOGGLERS)
	extern uint32_t __interruptsDisable;

	#define DISABLE_INTERRUPTS() serialprintf(COM1, "Trying to disable interrupts (%d)...\n", __interruptsDisable); __disableInterrupts()
	#define ENABLE_INTERRUPTS() serialprintf(COM1, "Trying to enable interrupts (%d)...\n", __interruptsDisable); __enableInterrupts()
#else
	#define DISABLE_INTERRUPTS() __disableInterrupts()
	#define ENABLE_INTERRUPTS() __enableInterrupts()
#endif

#define LOG(level, format, ...)		printf("[%s] " format "\n", level, ##__VA_ARGS__)
#define LOG_INFO(format, ...)		LOG("INFO", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)		LOG("WARN", format, ##__VA_ARGS__)
#define LOG_ERR(format, ...)		LOG("ERR", format, ##__VA_ARGS__)

#define BREAKPOINT {					\
	asm volatile("xchgw %bx, %bx;");	\
}

typedef struct {
	uint32_t x;
	uint32_t y;
} Vector2_32_t;

void 		memPrint(uint8_t* mem, uint32_t size);
void		outb(uint16_t port, uint8_t value);
void		outw(uint16_t port, uint16_t value);
void		outl(uint16_t port, uint32_t value);
uint8_t		inb(uint16_t port);
uint16_t	inw(uint16_t port);
uint32_t	inl(uint16_t port);

void		sleep(uint32_t ms);

void		kernel_warn(const char* message);
void		kernel_panic(const char* message);
void		kernel_assert(const char* message, const char* file, uint32_t line);
void		kernel_halt();