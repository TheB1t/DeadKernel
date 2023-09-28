#include <interrupts/isr.h>
#include <io/serial.h>
#include <multitasking/task.h>

InterruptHandler_t interruptHandlers[256];

uint8_t* getInterruptName(uint32_t i) {
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

		case IRQ0: return "System timer interrupt";
		case IRQ1: return "Keyboard interrupt";

		case CORE_FORK: return "Forking";
		case CORE_YIELD: return "Yielding";
		case CORE_SYSCALL: return "System call";

		default: return "Unknown exception";
	}
}

CPURegisters_t* interruptContext = NULL;
extern Task_t* currentTask;

void MainInterruptHandler(CPURegisters_t* regs) {
	interruptContext = regs;
	
	if (regs->int_no >= 32 && regs->int_no <= 47) {
		if (regs->int_no >= 40)
			outb(0xA0, 0x20);

		outb(0x20, 0x20);
	}

	// if (regs->int_no != 128 && regs->int_no != 32) {
	// 	serialprintf(COM1, "Interrupt (%d, %s)...\n", regs->int_no, getInterruptName(regs->int_no));
	// }

	if (interruptHandlers[regs->int_no] != 0) {
		interruptHandlers[regs->int_no](regs);
	} else if (regs->int_no >= 0 && regs->int_no <= 18) {
		uint8_t* name = getInterruptName(regs->int_no);
		LOG_WARN("Unhandled interrupt [%3d]: %s", regs->int_no, name);
		PANIC(name);
	}

	interruptContext = NULL;
}

CPURegisters_t* getInterruptedContext() {
	return interruptContext;
}

void registerInterruptHandler(uint8_t n, InterruptHandler_t handler) {
	interruptHandlers[n] = handler;
}

void unregisterInterruptHandler(uint8_t n) {
	interruptHandlers[n] = NULL;
}