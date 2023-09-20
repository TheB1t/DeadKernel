#pragma once

#include "common.h"
#include "screen.h"

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

typedef struct {
	uint32_t	CF   : 1;
	uint32_t	r0   : 1;
	uint32_t	PF   : 1;
	uint32_t	r1   : 1;
	uint32_t	AF   : 1;
	uint32_t	r2   : 1;
	uint32_t	ZF   : 1;
	uint32_t	SF   : 1;
	uint32_t	TF   : 1;
	uint32_t	IF   : 1;
	uint32_t	DF   : 1;
	uint32_t	OF   : 1;
	uint32_t	IOPL : 2;
	uint32_t	NT   : 1;
	uint32_t	MD   : 1;
	uint32_t	RF   : 1;
	uint32_t	VM   : 1;
	uint32_t	AC   : 1;
	uint32_t    VIF  : 1;
	uint32_t	VIP  : 1;
	uint32_t	ID   : 1;
	uint32_t    r3   : 10;
} CPUEFLAGS_t;

typedef struct {
	uint32_t	_esp;

	uint32_t	ds;
	uint32_t	cr3;
	uint32_t	edi;
	uint32_t	esi;
	uint32_t	ebx;
	uint32_t	edx;
	uint32_t	ecx;
	uint32_t	eax;
	uint32_t	ebp;

	uint32_t	int_no;
	uint32_t	err_code;

	// IRET Main
	uint32_t	eip;
	uint32_t	cs;
	CPUEFLAGS_t eflags;

	// IRET Second
	uint32_t	val0;
	uint32_t	val1;
} CPURegisters_t;

extern uint32_t interruptedEIP;
typedef void (*InterruptHandler_t)(CPURegisters_t* regs);
void registerInterruptHandler(uint8_t n, InterruptHandler_t handler);
void unregisterInterruptHandler(uint8_t n);
