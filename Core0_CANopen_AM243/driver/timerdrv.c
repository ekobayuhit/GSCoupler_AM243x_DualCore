#include "timerdrv.h"
#include <kernel/dpl/DebugP.h>

static TaskHandle_t TimerTask = NULL;

void timerInit(TaskHandle_t xTimerTask)
{
    TimerTask = xTimerTask;
}

void TimerTaskLoop(void *args)
{
    (void)args;

    for (;;)
    {
        /* Wait until resumed */
        vTaskSuspend(NULL);

        /* Run loop when resumed */
        while (1)
        {
            CANTimerCallBack();
            vTaskDelay(1);
        }
    }
}

void startTimerTask(void)
{
    if (TimerTask != NULL)
    {
        vTaskResume(TimerTask);
    }
}

void stopTimerTask(void)
{
    if (TimerTask != NULL)
    {
        vTaskSuspend(TimerTask);
    }
}

void StartTimerLoop(TimerCallback_t _init_callback)
{
    /* First dispatch will call init_callback */
    SetAlarm(NULL, 0, _init_callback, 0, 0);

    startTimerTask();
}