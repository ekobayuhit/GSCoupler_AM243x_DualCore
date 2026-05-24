#ifndef TIMERDRV_H
#define TIMERDRV_H

#include "FreeRTOS.h"
#include "task.h"

#include "applicfg.h"
#include "timer.h"

void timerInit(TaskHandle_t xTimerTask);
void startTimerTask(void);
void stopTimerTask(void);

void StartTimerLoop(TimerCallback_t _init_callback);
void TimerTaskLoop(void *args);

#endif