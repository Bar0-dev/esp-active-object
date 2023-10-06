#ifndef EVENTS_BROKER_H
#define EVENTS_BROKER_H

#include "esp_ao.h"

#define MAX_QUEUES_PER_EVENT 10

enum eventIds
{
    EV_BUTTON_PRESSED = USER_SIG,
    EV_BUTTON_RELEASED,
    EV_BUTTON_DOUBLE_PRESS,
    EV_BUTTON_HOLD,
    LAST_EVENT_FLAG,
};

typedef struct 
{
    QueueHandle_t queues[MAX_QUEUES_PER_EVENT];
    uint8_t queuesSubscribed;
} GlobalEvent;

typedef struct
{
    Active super;
    GlobalEvent globalEvents[LAST_EVENT_FLAG];
} Broker;

void Broker_ctor(Broker * const me);

void Broker_subscribe(Broker * const me, Event const * const e, QueueHandle_t queueHandle);

#endif