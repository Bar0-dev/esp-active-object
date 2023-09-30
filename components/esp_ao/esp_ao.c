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
static uint16_t l_activeTimers = 0;

static void xTimerCallback(TimerHandle_t xTimer)
{
    TimeEvent *timeEvent = (TimeEvent *)pvTimerGetTimerID(xTimer);
    assert(xTimer == timeEvent->handle);
    Active_post(timeEvent->act, &timeEvent->super);
}

void TimeEvent_ctor(TimeEvent * const me, char * const timerName, TickType_t period, UBaseType_t autoReload, Signal sig, Active *act)
{
    TimerHandle_t xTimer;
    
    me->super.sig = sig;
    me->act = act;
    xTimer = xTimerCreate(timerName, period, autoReload, (void *)0, xTimerCallback);
    assert(xTimer);
    l_timeEvents[l_activeTimers] = me;
    vTimerSetTimerID(xTimer, l_timeEvents[l_activeTimers]);
    l_activeTimers++;
    me->handle = xTimer;
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
    xReturned = xTimerStop(me->handle, (TickType_t)0);
    assert(xReturned);
}