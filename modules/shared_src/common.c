#include <common.h>
#include <syscall.h>

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

void printf(const char* format, ...) {
	char** arg = (char**)&format;
	char c, num = 0, sym = ' ';
	char buf[20];
	char padRight = 0;

	arg++;

	while ((c = *format++) != 0) {
		padRight = 0;
		sym = ' ';
		
		if (c != '%')
			screenPutChar(c);
		else {
			char* p;
			char* p2;
	
			c = *format++;
		back:
			switch (c) {
				case 'd':
				case 'u':
				case 'x':
					itoa(buf, c == 'x' ? 16 : 10, *((int*) arg++));
					p = buf;
					goto string;

				case 's':
					p = *arg++;
					if (!p)
						p = "(null)";

				string:
					if (num) {
						num -= strlen(p) > num ? num : strlen(p);
						if (!padRight)
							while (num--)
								screenPutChar(sym);
					}
					
					while (*p)
						screenPutChar(*p++);

					if (num && padRight)
						while (num--)
							screenPutChar(sym);
								
					break;

				case 'c':
					screenPutChar(*((int*) arg++));
				
				default:
					if (*(format - 2) == '%') {
						if (c == '0' || c == ' ' || c == '.') {
							sym = c;
							c = *format++;
						}
						if (c == '-') {
							padRight = 1;
							c = *format++;
						}
						num = 0;
						while (c >= '1' && c <= '9') {
							num *= 10;
							num += c - '0';
							c = *format++;
						}
						goto back;
					} else {
						screenPutChar(*((int*) arg++));
					}
					break;
			}
		}
	}
}
