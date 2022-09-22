#pragma once

#include "common.h"

extern uint32_t	screenGetColor();
extern void		screenSetColor(uint32_t color);
extern void		screenPutChar(uint8_t chr);
extern void		screenPutString(uint8_t* str);
extern void		screenClear();

extern uint32_t	getPID();

extern void yield();