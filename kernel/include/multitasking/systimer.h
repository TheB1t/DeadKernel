#pragma once

#include <utils/common.h>
#include <interrupts/isr.h>
#include <io/screen.h>
#include <multitasking/task.h>

void        initSysTimer();
uint32_t    getSysTimerTicks();
void        resetSysTimerTicks();
uint32_t    getUptime();