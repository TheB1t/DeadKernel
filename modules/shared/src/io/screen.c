#include <io/screen.h>

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
						screenPutChar(*((int*) arg++));
					}
					break;
			}
		}
	}
}
