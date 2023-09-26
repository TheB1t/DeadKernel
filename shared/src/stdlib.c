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

int32_t atoi(char* str) {
    int sign = 1, base = 0, i = 0;
 
    while (str[i] == ' ')
        i++;

    if (str[i] == '-' || str[i] == '+')
        sign = 1 - 2 * (str[i++] == '-');
 
    while (str[i] >= '0' && str[i] <= '9') {
        if (base > INT_MAX / 10 || (base == INT_MAX / 10 && str[i] - '0' > 7)) {
            if (sign == 1)
                return INT_MAX;
            else
                return INT_MIN;
        }
        base = 10 * base + (str[i++] - '0');
    }
    return base * sign;
}

bool isspace(char ch) {
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f');
}

char isdigit(char ch) {
    return (ch >= '0' && ch <= '9');
}

char tolower(char ch) {
    if (ch >= 'A' && ch <= 'Z')
        return ch + ('a' - 'A');

    return ch;
}

char toupper(char ch) {
    if (ch >= 'a' && ch <= 'z')
        return ch - ('a' - 'A');

    return ch;
}

int32_t strtoi(const char* str, char** endptr, int base) {
    if (str == NULL) {
        if (endptr != NULL)
            *endptr = NULL;

        return 0;
    }

    while (isspace(*str))
        str++;

    int sign = 1;
    if (*str == '+' || *str == '-') {
        if (*str == '-') {
            sign = -1;
        }
        str++;
    }

    if (base == 0) {
        if (*str == '0') {
            if (str[1] == 'x' || str[1] == 'X') {
                base = 16;
                str += 2;
            } else {
                base = 8;
                str++;
            }
        } else
            base = 10;
    } else if (base == 16) {
        if (*str == '0' && (str[1] == 'x' || str[1] == 'X'))
            str += 2;
    }

    int32_t result = 0;
    while (isdigit(*str) || (*str >= 'a' && *str <= 'f') || (*str >= 'A' && *str <= 'F')) {
        int digit;

        if (isdigit(*str))
            digit = *str - '0';
        else
            digit = tolower(*str) - 'a' + 10;

        if (digit >= base)
            break;

        result = result * base + digit;
        str++;
    }

    if (endptr != NULL)
        *endptr = (char *)str;

    return result * sign;
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