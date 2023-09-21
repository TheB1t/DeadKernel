#pragma once

#include <utils/common.h>
#include <memory_managment/kheap.h>
#include <interrupts/isr.h>
#include <multitasking/systimer.h>
#include <utils/bool.h>
#include <utils/cyclic_buffer.h>

#define KEYBOARD_COMMAND_BUFFER_SIZE 16
#define KEYBOARD_CHAR_BUFFER_SIZE 64

uint8_t initKeyboard();

bool keyboardReadReady();
uint8_t keyboardGetChar();
