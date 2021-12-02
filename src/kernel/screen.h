#pragma once

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#include "common.h"

typedef struct {
	uint8_t	foreground :4;
	uint8_t background :4;
} screenColor;

typedef struct {
	char c;
	screenColor color;
} screenChar;

void screenPutChar(char c);
void screenPutString(char* str);
void screenClear();

void printf(const char* format, ...);
