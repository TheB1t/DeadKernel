#include "syscall.h"

#include "screen.h"

extern uint32_t callSysCall(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void*);
static void SysCallHandler(registers_t regs);

static void* syscalls[] = {
	&screenPutChar,
	&screenPutString,
	&screenClear,
};

void initSysCalls() {
	registerInterruptHandler(128, &SysCallHandler);
}

void SysCallHandler(registers_t regs) {
	if (regs.eax >= sizeof(syscalls))
		return;

	void* location = syscalls[regs.eax];

	int ret;
	asm volatile ("	\
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
		:	"r" (regs.edi),
			"r" (regs.esi),
			"r" (regs.edx),
			"r" (regs.ecx),
			"r" (regs.ebx),
			"r" (location)
	);
	regs.eax = ret;
}

DEFN_SYSCALL1(screenPutChar, 0, char);
DEFN_SYSCALL1(screenPutString, 1, char*);
DEFN_SYSCALL0(screenClear, 2);
