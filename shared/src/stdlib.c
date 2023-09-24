#include <stdlib.h>

uint32_t getRing() {
	uint32_t ss;	asm volatile("mov %%ss, %0" : "=r" (ss));
	return ss & 3;
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

unsigned int is_delim(char c, char* delim) {
    while (*delim != '\0') {
        if (c == *delim)
            return 1;
        
        delim++;
    }

    return 0;
}

char* strtok(char* srcString, char* delim) {
    static char* backup_string;
    if (!srcString)
        srcString = backup_string;

    if (!srcString)
        return NULL;

    while (1) {
        if (is_delim(*srcString, delim)) {
            srcString++;
            continue;
        }

        if (*srcString == '\0')
            return NULL;

        break;
    }

    char *ret = srcString;
    while (1) {
        if (*srcString == '\0') {
            backup_string = srcString;
            return ret;
        }

        if (is_delim(*srcString, delim)) {
            *srcString = '\0';
            backup_string = srcString + 1;
            return ret;
        }
        srcString++;
    }
}