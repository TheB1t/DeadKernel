#pragma once

typedef	unsigned int	uint32_t;
typedef			 int	int32_t;
typedef	unsigned short	uint16_t;
typedef			 short	int16_t;
typedef	unsigned char	uint8_t;
typedef 		 char	int8_t;

#define NULL (void*)0

void 		memcpy(void* dest, const void* src, uint32_t len);
void 		memset(void* dest, uint8_t val, uint32_t len);
int			strlen(const char* str);
int 		strcmp(const char* str1, const char* str2);
char*		strcpy(char* dest, const char* src);
char*		strcat(char* dest, const char* src);
void		itoa(char* result, uint32_t base, int32_t value);
void		printf(const char* format, ...);
