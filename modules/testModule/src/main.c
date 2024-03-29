#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

int32_t main() {
	uint32_t esp;	asm volatile("mov %%esp, %0" : "=r" (esp));
	uint32_t ebp;	asm volatile("mov %%ebp, %0" : "=r" (ebp));

	uint32_t pid = getPID();
	printf("Welcome from ELF binary! PID %d ESP 0x%08x EBP 0x%08x Ring %d\n", pid, esp, ebp, getRing());

	for (uint32_t i = 0; i < 0x100; i++) {
		yield();
	};
	return 0xDEADBEEF;
}