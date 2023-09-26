#pragma once

#include <types.h>
#include <bool.h>
#include <math.h>
#include <limits.h>

#define BREAKPOINT {					\
	asm volatile("xchgw %bx, %bx;");	\
}

uint32_t    getRing();

void 		memcpy(void* dest, const void* src, uint32_t len);
void 		memset(void* dest, uint8_t val, uint32_t len);
int			strlen(const char* str);
int 		strcmp(const char* str1, const char* str2);
int			strncmp(const char *str1, const char *str2, uint32_t n);
char*		strcpy(char* dest, const char* src);
char*		strcat(char* dest, const char* src);
void		itoa(char* result, uint32_t base, int32_t value);
int32_t 	atoi(char* str);
bool		isspace(char ch);
char		isdigit(char ch);
char		tolower(char ch);
char		toupper(char ch);
int32_t		strtoi(const char* str, char** endptr, int base);
char* 		strtok(char* srcString, char* delim);