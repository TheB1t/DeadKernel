#pragma once

typedef	unsigned int	uint32_t;
typedef			 int	int32_t;
typedef	unsigned short	uint16_t;
typedef			 short	int16_t;
typedef	unsigned char	uint8_t;
typedef 		 char	int8_t;

#define NULL (void*)0

#define WARN(msg)	kernel_warn(msg)
#define PANIC(msg)	kernel_panic(msg)
#define ASSERT(b)	((b) ? (void)0 : kernel_assert(#b, __FILE__, __LINE__))

#define FPRINTF(f, ...)					\
{										\
	printf("%-16s: ", __FUNCTION__);	\
	printf(f, ##__VA_ARGS__);			\
}	

#define BREAKPOINT {					\
	asm volatile("xchgw %bx, %bx;");	\
}

typedef struct {
	uint32_t x;
	uint32_t y;
} Vector2_32_t;


void		outb(uint16_t port, uint8_t value);
void		outw(uint16_t port, uint16_t value);
void		outl(uint16_t port, uint32_t value);
uint8_t		inb(uint16_t port);
uint16_t	inw(uint16_t port);
uint32_t	inl(uint16_t port);
void 		memcpy(void* dest, const void* src, uint32_t len);
void 		memset(void* dest, uint8_t val, uint32_t len);
int			strlen(const char* str);
int 		strcmp(const char* str1, const char* str2);
char*		strcpy(char* dest, const char* src);
char*		strcat(char* dest, const char* src);
void		itoa(char* result, uint32_t base, int32_t value);

void		kernel_warn(const char* message);
void		kernel_panic(const char* message);
void		kernel_assert(const char* message, const char* file, uint32_t line);
