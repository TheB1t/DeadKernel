#include <utils/common.h>
#include <io/screen.h>
#include <interrupts/isr.h>
#include <multitasking/task.h>
#include <multitasking/systimer.h>
#include <utils/stackTrace.h>

void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value) {
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void outl(uint16_t port, uint32_t value) {
	asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

void memcpy(void *dest, const void *src, uint32_t len) {
    const uint8_t* sp = (const uint8_t*)src;
    uint8_t* dp = (uint8_t*)dest;
    while(len--) 
		*dp++ = *sp++;
}

void memset(void* dest, uint8_t val, uint32_t len) {
    uint8_t* temp = (uint8_t*)dest;
    while (len--) 
		*temp++ = val;
}

int strlen(const char* str) {
	int len = 0;
	while (*str++)
		len++;

	return len;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1) {
        if (*str1 != *str2)
            break;
 
        str1++;
        str2++;
    }
 
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

int strncmp(const char *str1, const char *str2, uint32_t n) {
    while (n > 0 && *str1) {
		if (*str1 != *str2)
			break;

        str1++;
        str2++;
        n--;
    }
 
    return n != 0;
}

char* strcpy(char* dest, const char* src) {
	char* tmp = dest;
	while((*dest++ = *src++) != '\0');
	*(--dest) = '\0';
	return tmp;
}

char *strcat(char* dest, const char* src) {
	char* tmp = dest;
	while(*dest)
		dest++;
	while((*dest++ = *src++) != '\0');
	return tmp;
}

void itoa(char* result, uint32_t base, int32_t value) {
    // check that the base if valid
    if (base < 2 || base > 36) {
        *result = '\0';
    }

    char *ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void sleep(uint32_t ms) {
	uint32_t targetTicks = getSysTimerTicks() + ms;
	while (getSysTimerTicks() < targetTicks);
}

void kernel_warn(const char* message) {
	printf("[Kernel Warning] %s\n", message);
}

uint8_t panicCounter = 0;

void kernel_panic(const char* message) {
	if (panicCounter >= 1) {
		printf("Double panic!\n");
		for(;;);
	}
	
	panicCounter++;
	DISABLE_INTERRUPTS();
	printf("[Kernel Panic] %s at address 0x%08x\n", message, getInterruptedContext()->eip);
	stackTrace(8);
	for(;;);
}

void kernel_assert(const char* message, const char* file, uint32_t line) {
	DISABLE_INTERRUPTS();
	printf("[Assertion Failed] %s (%s:%d)", message, file, line);
	for(;;);
}

void kernel_halt() {
	DISABLE_INTERRUPTS();
	uint8_t good = 0x02;
	while (good & 0x02)
		good = inb(0x64);
	outb(0x64, 0xFE);
loop:
	asm volatile("hlt");
	goto loop;
}
