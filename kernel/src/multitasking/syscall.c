#include <multitasking/syscall.h>
#include <interrupts/isr.h>
#include <io/screen.h>
#include <multitasking/task.h>

extern uint32_t callSysCall(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void*);
static void SysCallHandler();

static uint32_t syscallsCount = 0;
static void* syscalls[256];

void addSysCall(void* call) {
	if (syscallsCount >= sizeof(syscalls))
		return;
		
	syscalls[syscallsCount++] = call;
}

void initSysCalls() {
	memset(syscalls, 0, sizeof(syscalls));
	
	addSysCall(&screenGetColor);
	addSysCall(&screenSetColor);
	addSysCall(&screenPutChar);
	addSysCall(&screenPutString);
	addSysCall(&screenClear);

	addSysCall(&getPID);

	registerInterruptHandler(128, &SysCallHandler);
}

void SysCallHandler(CPURegisters_t* regs) {

	if (regs->eax >= syscallsCount)
		return;

	void* location = syscalls[regs->eax];

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
