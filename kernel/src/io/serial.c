#include <io/serial.h>

int32_t serialInit(uint16_t port, uint16_t baud) {
	outb(port + 1, 0x00);									// Disable all interrupts
	outb(port + UART_LCR,		UART_LCR_DLAB);				// Enable DLAB (set baud rate divisor)
	outb(port + UART_DL_LOW,	(baud & 0x00FF) >> 0);		// Set divisor to 3 (lo byte) 38400 baud
	outb(port + UART_DL_HIGH,	(baud & 0xFF00) >> 8);		// (hi byte)
	outb(port + UART_LCR,		UART_LCR_8BIT);				// 8 bits, no parity, one stop bit
	outb(port + UART_FCR,		0xC7);						// Enable FIFO, clear them, with 14-byte threshold
	outb(port + UART_MCR,		0x0B);						// IRQs enabled, RTS/DSR set
	outb(port + UART_MCR,		0x1E);						// Set in loopback mode, test the serial chip
	outb(port + UART_DAR,		0xAE);						// Test serial chip (send byte 0xAE and check if serial returns same byte)
	
	// Check if serial is faulty (i.e: not same byte as sent)
	if(inb(port + UART_DAR) != 0xAE)
		return 1;
 
	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(port + UART_MCR, 0x0F);
	return 0;
}

int32_t serialRecived(uint16_t port) {
	return inb(port + UART_LSR) & 1;
}
 
uint8_t serialReadByte(uint16_t port) {
   while (serialRecived(port) == 0);
   return inb(port);
}

uint32_t serialReadString(uint16_t port, uint8_t* str) {
	uint32_t bytesReaded = 0;
	while (1) {
		uint8_t byte = serialReadByte(port);
		str[bytesReaded++] = byte;
		if (byte == '\0')
			break;
	}
	return bytesReaded;
}

int isTransmitEmpty(uint16_t port) {
	return inb(port + UART_LSR) & 0x20;
}
 
void serialWriteByte(uint16_t port, uint8_t a) {
	while (isTransmitEmpty(port) == 0);
	outb(port, a);
}

uint32_t serialWriteString(uint16_t port, uint8_t* str) {
	uint32_t bytesWrited = 0;
	while (1) {
		uint8_t byte = str[bytesWrited++];
		serialWriteByte(port, byte);
		if (byte == '\0')
			break;
	}
	return bytesWrited;	
}

void serialprintf(uint16_t port, const char* format, ...) {
	char** arg = (char**)&format;
	char c, num = 0, sym = ' ';
	char buf[20];
	char padRight = 0;

	arg++;

	while ((c = *format++) != 0) {
		padRight = 0;
		sym = ' ';
		
		if (c != '%')
			serialWriteByte(port, c);
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
								serialWriteByte(port, sym);
					}
					
					while (*p)
						serialWriteByte(port, *p++);

					if (num && padRight)
						while (num--)
							serialWriteByte(port, sym);
								
					break;

				case 'c':
					serialWriteByte(port, *((int*) arg++));
					break;
				
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
						serialWriteByte(port, *((int*) arg++));
					}
					break;
			}
		}
	}
}
