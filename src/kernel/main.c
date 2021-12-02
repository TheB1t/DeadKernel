#include "multiboot.h"
#include "common.h"
#include "screen.h"
#include "descriptor_tables.h"
#include "systimer.h"
#include "paging.h"
#include "kheap.h"

int main(multiboot_t* mboot) {
	screenClear();
	printf("Hello, world!\n");
	printf("Welcome to kernel!\n");

	initDescriptorTables();
	uint32_t a = kmalloc(8);

	initPaging();
	
	uint32_t b = kmalloc(8);
	uint32_t c = kmalloc(8);

	printf("a: 0x%x, b: 0x%x\n", a, b);

	kfree(c);
	kfree(b);
	uint32_t d = kmalloc(12);
	
	printf("c: 0x%x, d: 0x%x\n", c, d);
	
//	uint32_t* ptr = (uint32_t*)0xA0000000;
//   	uint32_t do_page_fault = *ptr;
//	asm volatile ("int $0x38");
	
//	initSysTimer(50);
	
	while (1) {
		
	}
	
	return 0xDEADBABA;
}
