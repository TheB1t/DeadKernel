#pragma once

typedef	unsigned int	uint32_t;
typedef			 int	int32_t;
typedef	unsigned short	uint16_t;
typedef			 short	int16_t;
typedef	unsigned char	uint8_t;
typedef 		 char	int8_t;

#define NULL (void*)0

#define GDT_DESC_SEG(n, cpl)	(uint32_t)(((uint32_t)0x8 * (uint32_t)n) | (uint32_t)cpl)

#define HALT		kernel_halt()
#define WARN(msg)	kernel_warn(msg)
#define PANIC(msg)	kernel_panic(msg)
#define ASSERT(b)	((b) ? (void)0 : kernel_assert(#b, __FILE__, __LINE__))

static uint32_t _interruptsDisable = 0;

#define DISABLE_INTERRUPTS() {				\
	if (_interruptsDisable == 0)			\
		asm volatile("cli");				\
	_interruptsDisable++;					\
}											\

#define ENABLE_INTERRUPTS() {				\
	_interruptsDisable--;					\
	if (_interruptsDisable == 0)			\
		asm volatile("sti");				\
}											\

#define LOG(level, format, ...)		printf("[%s] " format "\n", level, ##__VA_ARGS__)
#define LOG_INFO(format, ...)		LOG("INFO", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)		LOG("WARN", format, ##__VA_ARGS__)
#define LOG_ERR(format, ...)		LOG("ERR", format, ##__VA_ARGS__)

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
int			strncmp(const char *str1, const char *str2, uint32_t n);
char*		strcpy(char* dest, const char* src);
char*		strcat(char* dest, const char* src);
void		itoa(char* result, uint32_t base, int32_t value);

void		sleep(uint32_t ms);

void		kernel_warn(const char* message);
void		kernel_panic(const char* message);
void		kernel_assert(const char* message, const char* file, uint32_t line);
void		kernel_halt();