#include "events_broker.h"
#include "esp_log.h"

static GlobalEvent globalEvents[LAST_EVENT_FLAG];

static void Broker_publish(Broker * const me, Event const * const e)
{
    if (e->sig != INIT_SIG)
    {
        uint8_t eventId = e->sig-1;
        GlobalEvent *globalEvent = &globalEvents[eventId];
        for (int q = 0; q<globalEvent->queuesSubscribed; q++)
        {
            assert(xQueueSend(globalEvent->queues[q], &(Event){ e->sig }, (TickType_t)10));
        }
    }
}

void Broker_ctor(Broker * const me)
{
    Active_ctor(&me->super, (DispatchHandler)&Broker_publish);
}

void Broker_subscribe(Broker * const me, Event const * const e, QueueHandle_t queueHandle)
{
    uint8_t eventId = e->sig-1;
    GlobalEvent *globalEvent = &globalEvents[eventId];
    globalEvent->queues[globalEvent->queuesSubscribed] = queueHandle;
    globalEvent->queuesSubscribed++;
}