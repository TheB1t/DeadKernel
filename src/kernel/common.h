#pragma once

typedef	unsigned int	uint32_t;
typedef			 int	int32_t;
typedef	unsigned short	uint16_t;
typedef			 short	int16_t;
typedef	unsigned char	uint8_t;
typedef 		 char	int8_t;

#define PANIC(msg)	kernel_panic(msg, __FILE__, __LINE__)
#define ASSERT(b)	((b) ? (void)0 : kernel_assert(#b, __FILE__, __LINE__))

typedef struct {
	uint32_t x;
	uint32_t y;
} Vector2_32_t;

void		outb(uint16_t port, uint8_t value);
uint8_t		inb(uint16_t port);
uint16_t	inw(uint16_t port);
void 		memcpy(void* dest, const void* src, uint32_t len);
void 		memset(void* dest, uint8_t val, uint32_t len);
int 		strcmp(const char* str1, const char* str2);
char*		strcpy(char* dest, const char* src);
char*		strcat(char* dest, const char* src);
void		itoa(char* result, uint32_t base, int32_t value);

void		kernel_panic(const char* message, const char* file, uint32_t line);
void		kernel_assert(const char* message, const char* file, uint32_t line);
