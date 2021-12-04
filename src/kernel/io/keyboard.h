#pragma once

#include "common.h"
#include "kheap.h"
#include "isr.h"

#define MAX_KEYBOARD_BUFFER_SIZE 1024

uint8_t initKeyboard(uint32_t bufferSize);
uint8_t isKeyboardInit();
uint8_t getch();
