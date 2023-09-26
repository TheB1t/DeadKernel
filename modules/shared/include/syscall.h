#pragma once

#include <stdlib.h>
#include <drivers/pci/pci_types.h>

extern uint32_t	getPID();
extern int32_t	fork();
extern void		yield();

extern uint32_t	screenGetColor();
extern void		screenSetColor(uint32_t color);
extern void		screenPutChar(uint8_t chr);
extern void		screenPutString(uint8_t* str);
extern void		screenClear();

extern bool		keyboardReadReady();
extern char		keyboardGetChar();

uint32_t        malloc(uint32_t size);
void            free(void* addr);

uint32_t        PCIDirectScan(PCIDevice_t* devices);