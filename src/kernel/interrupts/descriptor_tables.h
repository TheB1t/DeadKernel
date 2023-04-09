#pragma once

#include "common.h"
#include "isr.h"

#define PL_RING0 0b00
#define PL_RING1 0b01
#define PL_RING2 0b10
#define PL_RING3 0b11

#define GDT_DESC_KERNEL_CODE	1
#define GDT_DESC_KERNEL_DATA	2

#define GDT_DESC_USER_CODE		3
#define GDT_DESC_USER_DATA		4

#define GDT_DESC_TSS0_DATA		5
#define GDT_DESC_TSS0   		6

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

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr64();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
