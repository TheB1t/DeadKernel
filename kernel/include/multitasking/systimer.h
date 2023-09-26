#pragma once

#include <utils/common.h>
#include <multitasking/task.h>
#include <interrupts/isr.h>

void        initSysTimer();
uint32_t    getSysTimerTicks();
void        resetSysTimerTicks();
uint32_t    getUptime();