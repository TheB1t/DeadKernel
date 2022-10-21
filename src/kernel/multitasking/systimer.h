#pragma once

#include "common.h"
#include "isr.h"
#include "screen.h"
#include "task.h"

void        initSysTimer();
uint32_t    getSysTimerTicks();
void        resetSysTimerTicks();
uint32_t    getUptime();