#include "screen.h"

static screenChar* framebuffer = (screenChar*)0xB8000; 

Vector2_32_t cursor			= { .x = 0, .y = 0 };
screenColor currentColor	= { .foreground = 15, .background = 0 };
screenChar blank			= { .c = 0x20, .color = { .foreground = 15, .background = 0 } };

screenColor screenGetColor() {
	return currentColor;
}

void screenSetColor(screenColor color) {
	currentColor = color;
}

static void moveCursor() {
	uint16_t cursorLocation = (cursor.y * SCREEN_WIDTH) + cursor.x;
	outb(0x3D4, 14);
	outb(0x3D5, cursorLocation >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, cursorLocation);
}

static void scroll() {
	if (cursor.y >= SCREEN_HEIGHT) {
		for (uint32_t i = 0 * SCREEN_WIDTH; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
			framebuffer[i] = framebuffer[i + SCREEN_WIDTH];
		}

		for (uint32_t i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
			framebuffer[i] = blank;
		}

		cursor.y = SCREEN_HEIGHT - 1;
	}
}

void screenPutChar(char c) {
	switch (c) {
		case '\b':
			if (cursor.x)
				cursor.x--;
				framebuffer[(cursor.y * SCREEN_WIDTH) + cursor.x] = blank;
			break;

		case '\t':
			cursor.x = (cursor.x + (4 - (cursor.x % 4)));// & ~(8 - 1);
			break;

		case '\n':
			cursor.y++;
			
		case '\r':
			cursor.x = 0;
			break;

		default:
			if (c >= ' ') {
				screenChar* sc = &framebuffer[(cursor.y * SCREEN_WIDTH) + cursor.x];
				sc->color = currentColor;
				sc->c = c;
				cursor.x++;
			}
	}

	if (cursor.x >= 80) {
		cursor.x = 0;
		cursor.y++;
	}

	scroll();
	moveCursor();
}

void screenClear() {
	for (uint32_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		framebuffer[i] = blank;
	}

	cursor.x = 0;
	cursor.y = 0;
	moveCursor();
}

void screenPutString(char* c) {
	while (*c) {
		screenPutChar(*c++);
	}
}

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
