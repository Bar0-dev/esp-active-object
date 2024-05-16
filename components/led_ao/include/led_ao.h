#ifndef LED_AO_H
#define LED_AO_H

#include "esp_ao.h"
#include "events_broker.h"

#define LED_PIN 27

typedef struct
{
    Active super;
    uint16_t blinkPeriod;
    TimeEvent ledTimer;
} Led;

enum LedEventSignals
{
    BLINK_TIMER_EXPIRED_SIG = LAST_EVENT_FLAG,
};

void Led_ctor(Led * const me);

#endif