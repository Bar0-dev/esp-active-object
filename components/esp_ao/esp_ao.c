#include <stdio.h>
#include "esp_ao.h"
#include "freertos/timers.h"

#include "esp_log.h"


void Active_ctor(Active * const me, DispatchHandler dispatch)
{
    me->dispatch = dispatch;
}

static void Active_eventLoop(void *pvParameters)
{
    Active *me = (Active *)pvParameters;

    static Event const initEvent = { INIT_SIG };
    (*me->dispatch)(me, &initEvent);

    while (1)
    {
        BaseType_t xReturned;
        Event e;

        xReturned = xQueueReceive(me->queue, (void *)&e, (TickType_t)10);
        // assert(xReturned == pdPASS || xReturned == pdFAIL);
        if(xReturned == pdPASS)
        {
            (*me->dispatch)(me, &e);
        }
    }   
}

void Active_start(Active * const me,
                const char *const taskNamePtr,
                const uint32_t stackSize, /*stack size in bytes*/
                UBaseType_t taskPriority,
                const BaseType_t core, /*core the task should run on, default tskNO_AFFINITY enum*/
                UBaseType_t queueLength)
{
    BaseType_t xReturned;
    assert(me && (taskPriority > 0));

    me->queue = xQueueCreate(queueLength, sizeof(Event));
    assert(me->queue);

    xReturned = xTaskCreatePinnedToCore(&Active_eventLoop, taskNamePtr, stackSize, me, taskPriority, me->task, core);
    assert(xReturned == pdPASS);
}

void Active_post(Active * const me, Event const * const e)
{
    BaseType_t xReturned;
    xReturned = xQueueSend(me->queue, e, (TickType_t)10);
    assert(xReturned == pdPASS);
}

static TimeEvent *l_timeEvents[MAX_TIMERS];
static uint32_t l_activeTimeEvents = 0;

static void xTimerCallback(TimerHandle_t xTimer)
{
    TimeEvent *timeEvent;
    //portENTER_CRITICAL();
    for (int timeEventNum = 0; timeEventNum<l_activeTimeEvents; timeEventNum++)
    {
        timeEvent = l_timeEvents[timeEventNum];
        if(xTimer == timeEvent->handle)
        {
            Active_post(timeEvent->act, &timeEvent->super);
        }
    }
    //portEXIT_CRITICAL();
}

void TimeEvent_ctor(TimeEvent * const me, char * const timerName, TickType_t period, UBaseType_t autoReload, Signal sig, Active *act)
{
    TimerHandle_t xTimer;
    
    me->super.sig = sig;
    me->act = act;
    xTimer = xTimerCreate(timerName, period, autoReload, (void *)0, xTimerCallback);
    assert(xTimer);
    me->handle = xTimer;
    //portENTER_CRITICAL();
    l_timeEvents[l_activeTimeEvents] = me;
    l_activeTimeEvents++;
    //portEXIT_CRITICAL();
}

void TimeEvent_arm(TimeEvent * const me)
{
    BaseType_t xReturned;
    xReturned = xTimerStart(me->handle, (TickType_t)0);
    assert(xReturned);
}

void TimeEvent_disarm(TimeEvent * const me)
{
    BaseType_t xReturned;
    //portENTER_CRITICAL();
    xReturned = xTimerStop(me->handle, (TickType_t)0);
    assert(xReturned);
    l_timeEvents[l_activeTimeEvents] = NULL;
    l_activeTimeEvents--;
    //portEXIT_CRITICAL();
}