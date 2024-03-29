#pragma once

#include <utils/common.h>

#define UART_DAR		0
#define UART_IER		1
#define UART_DL_LOW		0
#define UART_DL_HIGH	1
#define UART_FCR		2
#define UART_LCR		3
#define UART_MCR		4
#define UART_LSR		5
#define UART_MSR		6

#define UART_LCR_DLAB	0b10000000
#define UART_LCR_5BIT	0b00000000
#define UART_LCR_6BIT	0b00000001
#define UART_LCR_7BIT	0b00000010
#define UART_LCR_8BIT	0b00000011

#define UART_BAUD_600		0x00C0
#define UART_BAUD_1200		0x0060
#define UART_BAUD_1800		0x0040
#define UART_BAUD_2000		0x003A
#define UART_BAUD_2400		0x0030
#define UART_BAUD_3600		0x0020
#define UART_BAUD_4800		0x0018
#define UART_BAUD_7200		0x0010
#define UART_BAUD_9600		0x000C
#define UART_BAUD_19200		0x0006
#define UART_BAUD_38400		0x0003
#define UART_BAUD_57600		0x0002
#define UART_BAUD_115200	0x0001

#define COM1 0x3F8

#define SERIAL_LOG(level, port, format, ...)		serialprintf(port, "[%s] " format "\n", level, ##__VA_ARGS__)
#define SERIAL_LOG_INFO(port, format, ...)		    SERIAL_LOG("INFO", port, format, ##__VA_ARGS__)
#define SERIAL_LOG_WARN(port, format, ...)		    SERIAL_LOG("WARN", port, format, ##__VA_ARGS__)
#define SERIAL_LOG_ERR(port, format, ...)		    SERIAL_LOG("ERR", port, format, ##__VA_ARGS__)
 
int32_t		serialInit(uint16_t port, uint16_t baud);
uint8_t		serialReadByte(uint16_t port);
uint32_t	serialReadString(uint16_t port, uint8_t* str);
void		serialWriteByte(uint16_t port, uint8_t a);
uint32_t	serialWriteString(uint16_t port, uint8_t* str);

void		serialprintf(uint16_t port, const char* format, ...);