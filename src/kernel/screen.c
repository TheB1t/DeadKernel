#include "screen.h"

static screenChar* framebuffer = (screenChar*)0xB8000; 

Vector2_32_t cursor			= { .x = 0, .y = 0 };
screenColor currentColor	= { .foreground = 15, .background = 0 };
screenChar blank			= { .c = 0x20, .color = { .foreground = 15, .background = 0 } };

static void moveCursor() {
	uint16_t cursorLocation = (cursor.y * SCREEN_WIDTH) + cursor.x;
	outb(0x3D4, 14);
	outb(0x3D4, cursorLocation >> 8);
	outb(0x3D4, 15);
	outb(0x3D4, cursorLocation);
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
			break;

		case '\t':
			cursor.x = (cursor.x + 8) & ~(8 - 1);
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
	int c;
	char buf[20];

	arg++;

	while ((c = *format++) != 0) {
		if (c != '%')
			screenPutChar(c);
		else {
			char* p;
			char* p2;
			
			c = *format++;

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
					while (*p)
						screenPutChar(*p++);
					break;

				default:
					screenPutChar(*((int*) arg++));
					break;
			}
		}
	}
}
