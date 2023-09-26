#include <multitasking/syscall.h>
#include <interrupts/isr.h>
#include <io/screen.h>
#include <multitasking/task.h>
#include <io/keyboard.h>
#include <memory_managment/user_heap.h>
#include <drivers/pci/pci_user.h>
#include <multitasking/semaphore.h>

extern uint32_t callSysCall(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void*);
static void SysCallHandler();

static void* syscalls[256];

void addSysCall(uint32_t num, void* call) {
	if (num >= sizeof(syscalls))
		return;
		
	syscalls[num] = call;
}

void initSysCalls() {
	memset(syscalls, 0, sizeof(syscalls));
	
	addSysCall(0x0, &getPID);
	addSysCall(0x1, &yield);
	addSysCall(0x2, &fork);

	addSysCall(0x3, &screenGetColor);
	addSysCall(0x4, &screenSetColor);
	addSysCall(0x5, &screenPutChar);
	addSysCall(0x6, &screenPutString);
	addSysCall(0x7, &screenClear);

    addSysCall(0x8, &keyboardReadReady);
    addSysCall(0x9, &keyboardGetChar);

    addSysCall(0xa, &user_malloc);
    addSysCall(0xb, &user_free);

	addSysCall(0xc, &user_PCIDirectScan);

	addSysCall(0xd, &semctl);

	registerInterruptHandler(128, &SysCallHandler);
}

void SysCallHandler(CPURegisters_t* regs) {
	void* location = syscalls[regs->eax];

	if (!location)
		return;
	int ret;
	asm volatile ("		\
		push %1;		\
		push %2;		\
		push %3;		\
		push %4;		\
		push %5;		\
		call *%6;		\
		pop %%ebx;		\
		pop %%ebx;		\
		pop %%ebx;		\
		pop %%ebx;		\
		pop %%ebx;		"
		:	"=a" (ret) 
		:	"r" (regs->edi),
			"r" (regs->esi),
			"r" (regs->edx),
			"r" (regs->ecx),
			"r" (regs->ebx),
			"r" (location)
	);
	regs->eax = ret;
}
