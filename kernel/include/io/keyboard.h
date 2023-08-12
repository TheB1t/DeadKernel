#pragma once

#include <utils/common.h>
#include <memory_managment/kheap.h>
#include <interrupts/isr.h>

#define MAX_KEYBOARD_BUFFER_SIZE 1024

uint8_t initKeyboard(uint32_t bufferSize);
uint8_t isKeyboardInit();
uint8_t getch();
