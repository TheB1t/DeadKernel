#pragma once

#include <utils/common.h>
#include <interrupts/isr.h>

#define PL_RING0 0b00
#define PL_RING1 0b01
#define PL_RING2 0b10
#define PL_RING3 0b11

#define GDT_DESC_KERNEL_CODE	1
#define GDT_DESC_KERNEL_DATA	2

#define GDT_DESC_USER_CODE		3
#define GDT_DESC_USER_DATA		4

#define GDT_DESC_TSS0   		5

struct GDTEntry {
	uint16_t	limit_low;
	uint16_t	base_low;
	uint8_t		base_middle;
	uint8_t		access;
	uint8_t		granularity;
	uint8_t		base_high;	
} __attribute__((packed));

struct GDTPTR {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct IDTEntry {
	uint16_t base_low;
	uint16_t sel;
	uint8_t	 always0;
	uint8_t  flags;
	uint16_t base_high;
} __attribute__((packed));

struct IDTPTR {
	uint16_t limit;
	uint32_t base;	
} __attribute__((packed));

typedef struct GDTEntry		GDTEntry_t;
typedef struct GDTPTR		GDTPTR_t;
typedef struct IDTEntry		IDTEntry_t;
typedef	struct IDTPTR		IDTPTR_t;

struct TSSEntry {
	uint32_t	prevTSS;
	uint32_t	esp0;
	uint32_t	ss0;
	uint32_t	esp1;
	uint32_t	ss1;
	uint32_t	esp2;
	uint32_t	ss2;
	uint32_t	cr3;
	uint32_t	eip;
	uint32_t	eflags;
	uint32_t	eax;
	uint32_t	ecx;
	uint32_t	edx;
	uint32_t	ebx;
	uint32_t	esp;
	uint32_t	ebp;
	uint32_t	esi;
	uint32_t	edi;
	uint32_t	es;
	uint32_t	cs;
	uint32_t	ss;
	uint32_t	ds;
	uint32_t	fs;
	uint32_t	gs;
	uint32_t	ldt;
	uint32_t	trap;
	uint32_t	iomap_base;
} __attribute__((packed));

typedef struct TSSEntry TSSEntry_t;

void initDescriptorTables();
void setKernelStack(uint32_t stack);
uint32_t getKernelStack();

#define INTERRUPT_ENTRY(t, v)	extern void t##v();

INTERRUPT_ENTRY(isr, 0);
INTERRUPT_ENTRY(isr, 1);
INTERRUPT_ENTRY(isr, 2);
INTERRUPT_ENTRY(isr, 3);
INTERRUPT_ENTRY(isr, 4);
INTERRUPT_ENTRY(isr, 5);
INTERRUPT_ENTRY(isr, 6);
INTERRUPT_ENTRY(isr, 7);
INTERRUPT_ENTRY(isr, 8);
INTERRUPT_ENTRY(isr, 9);
INTERRUPT_ENTRY(isr, 10);
INTERRUPT_ENTRY(isr, 11);
INTERRUPT_ENTRY(isr, 12);
INTERRUPT_ENTRY(isr, 13);
INTERRUPT_ENTRY(isr, 14);
INTERRUPT_ENTRY(isr, 15);
INTERRUPT_ENTRY(isr, 16);
INTERRUPT_ENTRY(isr, 17);
INTERRUPT_ENTRY(isr, 18);
INTERRUPT_ENTRY(isr, 19);
INTERRUPT_ENTRY(isr, 20);
INTERRUPT_ENTRY(isr, 21);
INTERRUPT_ENTRY(isr, 22);
INTERRUPT_ENTRY(isr, 23);
INTERRUPT_ENTRY(isr, 24);
INTERRUPT_ENTRY(isr, 25);
INTERRUPT_ENTRY(isr, 26);
INTERRUPT_ENTRY(isr, 27);
INTERRUPT_ENTRY(isr, 28);
INTERRUPT_ENTRY(isr, 29);
INTERRUPT_ENTRY(isr, 30);
INTERRUPT_ENTRY(isr, 31);

INTERRUPT_ENTRY(isr, 126);
INTERRUPT_ENTRY(isr, 127);
INTERRUPT_ENTRY(isr, 128);

INTERRUPT_ENTRY(irq, 0);
INTERRUPT_ENTRY(irq, 1);
INTERRUPT_ENTRY(irq, 2);
INTERRUPT_ENTRY(irq, 3);
INTERRUPT_ENTRY(irq, 4);
INTERRUPT_ENTRY(irq, 5);
INTERRUPT_ENTRY(irq, 6);
INTERRUPT_ENTRY(irq, 7);
INTERRUPT_ENTRY(irq, 8);
INTERRUPT_ENTRY(irq, 9);
INTERRUPT_ENTRY(irq, 10);
INTERRUPT_ENTRY(irq, 11);
INTERRUPT_ENTRY(irq, 12);
INTERRUPT_ENTRY(irq, 13);
INTERRUPT_ENTRY(irq, 14);
INTERRUPT_ENTRY(irq, 15);