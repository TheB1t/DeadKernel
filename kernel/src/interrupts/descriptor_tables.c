#include <interrupts/descriptor_tables.h>


#define pack_access(Type, DT, DPL, P) ((((0x00 | (Type & 0b1111)) | (DT & 0b1) << 4) | (DPL & 0b11) << 5) | (P & 0b1) << 7)
#define pack_granularity(A, D, G) (((0x00 | (A & 0b1) << 4) | (D & 0b1) << 6) | (G & 0b1) << 7)
#define pack_flags(P, DPL) ((((0x00 | 0b01110)) | (DPL & 0b11) << 5) | (P & 0b1) << 7)

//Global Description Table
#define GDT_ENTRIES_SIZE 6

GDTEntry_t	GDTEntries[GDT_ENTRIES_SIZE];
GDTPTR_t	GDT;

extern void GDTFlush(uint32_t);

static void InitGDT();
static void GDTSetGate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);

//Interrupt Description Table
#define IDT_ENTRIES_SIZE 256 

IDTEntry_t	IDTEntries[IDT_ENTRIES_SIZE];
IDTPTR_t	IDT;

extern void IDTFlush(uint32_t);

static void InitIDT();
static void IDTSetGate(uint8_t, uint32_t, uint16_t, uint8_t);

TSSEntry_t	TSSEntry0;

extern void TSSFlush(uint32_t);
static void writeTSS(int32_t, uint16_t, uint32_t);

extern InterruptHandler_t interruptHandlers[];

void initDescriptorTables() {
	DISABLE_INTERRUPTS();
	InitGDT();
	InitIDT();
	memset(&interruptHandlers, 0, sizeof(InterruptHandler_t) * IDT_ENTRIES_SIZE);
	ENABLE_INTERRUPTS();
}

static void InitGDT() {
	GDT.limit 	= (sizeof(GDTEntry_t) * GDT_ENTRIES_SIZE) - 1;
	GDT.base	= (uint32_t)&GDTEntries;
	
	GDTSetGate(0					, 0, 0, 0, 0);																				//Null segment			(0x00)
	GDTSetGate(GDT_DESC_KERNEL_CODE	, 0, 0xFFFFFFFF, pack_access(0b1010, 0b1, PL_RING0, 0b1), pack_granularity(0b0, 0b1, 0b1)); //Kernel Code segment	(0x08)
	GDTSetGate(GDT_DESC_KERNEL_DATA	, 0, 0xFFFFFFFF, pack_access(0b0010, 0b1, PL_RING0, 0b1), pack_granularity(0b0, 0b1, 0b1));	//Kernel Data segment	(0x10)
	GDTSetGate(GDT_DESC_USER_CODE	, 0, 0xFFFFFFFF, pack_access(0b1010, 0b1, PL_RING3, 0b1), pack_granularity(0b0, 0b1, 0b1)); //User Code segment		(0x18)
	GDTSetGate(GDT_DESC_USER_DATA	, 0, 0xFFFFFFFF, pack_access(0b0010, 0b1, PL_RING3, 0b1), pack_granularity(0b0, 0b1, 0b1)); //User Data segment 	(0x20)

	writeTSS(GDT_DESC_TSS0, GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING0), 0x0);
	
	GDTFlush((uint32_t)&GDT);
	TSSFlush(GDT_DESC_SEG(GDT_DESC_TSS0, PL_RING3));
}

static void GDTSetGate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
	GDTEntries[num].base_low		= (base & 0xFFFF);
	GDTEntries[num].base_middle		= (base >> 16) & 0xFF;
	GDTEntries[num].base_high		= (base >> 24) & 0xFF;

	GDTEntries[num].limit_low		= (limit & 0xFFFF);
	GDTEntries[num].granularity		= (limit >> 16) & 0x0F;

	GDTEntries[num].granularity		|= granularity & 0xF0;
	GDTEntries[num].access			= access;
}

static void InitIDT() {
	IDT.limit	= sizeof(IDTEntry_t) * IDT_ENTRIES_SIZE - 1;
	IDT.base	= (uint32_t)&IDTEntries;

	memset(&IDTEntries, 0, sizeof(IDTEntry_t) * IDT_ENTRIES_SIZE);

	IDTSetGate(0, (uint32_t)isr0, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(1, (uint32_t)isr1, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(2, (uint32_t)isr2, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(3, (uint32_t)isr3, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(4, (uint32_t)isr4, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(5, (uint32_t)isr5, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(6, (uint32_t)isr6, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(7, (uint32_t)isr7, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(8, (uint32_t)isr8, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(9, (uint32_t)isr9, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(10, (uint32_t)isr10, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(11, (uint32_t)isr11, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(12, (uint32_t)isr12, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(13, (uint32_t)isr13, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(14, (uint32_t)isr14, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(15, (uint32_t)isr15, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(16, (uint32_t)isr16, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(17, (uint32_t)isr17, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(18, (uint32_t)isr18, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(19, (uint32_t)isr19, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(20, (uint32_t)isr20, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(21, (uint32_t)isr21, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(22, (uint32_t)isr22, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(23, (uint32_t)isr23, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(24, (uint32_t)isr24, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(25, (uint32_t)isr25, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(26, (uint32_t)isr26, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(27, (uint32_t)isr27, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(28, (uint32_t)isr28, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(29, (uint32_t)isr29, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(30, (uint32_t)isr30, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(31, (uint32_t)isr31, 0x08, pack_flags(0b1, 0b00));
	
	IDTSetGate(64, (uint32_t)isr64, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(128, (uint32_t)isr128, 0x08, pack_flags(0b1, 0b00));
	
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	IDTSetGate(32, (uint32_t)irq0, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(33, (uint32_t)irq1, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(34, (uint32_t)irq2, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(35, (uint32_t)irq3, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(36, (uint32_t)irq4, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(37, (uint32_t)irq5, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(38, (uint32_t)irq6, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(39, (uint32_t)irq7, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(40, (uint32_t)irq8, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(41, (uint32_t)irq9, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(42, (uint32_t)irq10, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(43, (uint32_t)irq11, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(44, (uint32_t)irq12, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(45, (uint32_t)irq13, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(46, (uint32_t)irq14, 0x08, pack_flags(0b1, 0b00));
	IDTSetGate(47, (uint32_t)irq15, 0x08, pack_flags(0b1, 0b00));
	
	IDTFlush((uint32_t)&IDT);
}

static void IDTSetGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	IDTEntries[num].base_low	= (base & 0xFFFF);
	IDTEntries[num].base_high	= (base >> 16) & 0xFFFF;

	IDTEntries[num].sel			= sel;
	IDTEntries[num].always0		= 0;

	IDTEntries[num].flags		= flags | 0x60;
}

static void writeTSS(int32_t num, uint16_t ss0, uint32_t esp0) {
	uint32_t base	= (uint32_t)&TSSEntry0;
	uint32_t limit	= base + sizeof(TSSEntry_t);
	
	GDTSetGate(num, base, limit, pack_access(0b1001, 0b0, PL_RING3, 0b1), pack_granularity(0b0, 0b0, 0b0));

	memset(&TSSEntry0, 0, sizeof(TSSEntry_t));

	TSSEntry0.ss0	= ss0;
	TSSEntry0.esp0	= esp0;

	TSSEntry0.cs	= GDT_DESC_SEG(GDT_DESC_KERNEL_CODE, PL_RING3);
	TSSEntry0.ss	= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING3);
	TSSEntry0.ds	= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING3);
	TSSEntry0.es	= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING3);
	TSSEntry0.fs	= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING3);
	TSSEntry0.gs	= GDT_DESC_SEG(GDT_DESC_KERNEL_DATA, PL_RING3);
}

void setKernelStack(uint32_t stack) {
	TSSEntry0.esp0 = stack;
}

uint32_t getKernelStack() {
	return TSSEntry0.esp0;
}

