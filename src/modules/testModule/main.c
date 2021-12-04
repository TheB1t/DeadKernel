#include "common.h"
#include "syscall.h"

int32_t main() {
	asm volatile("cli");
	uint32_t esp;	asm volatile("mov %%esp, %0" : "=r" (esp));
	uint32_t ebp;	asm volatile("mov %%ebp, %0" : "=r" (ebp));
	asm volatile("sti");

	uint32_t pid = getPID();
	printf("Welcome from ELF binary! PID %d ESP 0x%08x EBP 0x%08x\n", pid, esp, ebp);

/*	for (uint32_t i = 0; i < 0xFFFFFFFF; i++) {
	//	printf("2");
	};
	printf("DONE!");
*/	return 0xDEADBEEF;
}
