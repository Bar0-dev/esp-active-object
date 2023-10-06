#ifndef LED_AO_H
#define LED_AO_H

#include "esp_ao.h"
#include "events_broker.h"

#define LED_PIN 27
#define BLINK_PERIOD_DEFAULT 200

typedef struct
{
    Active super;

    bool state;
    uint16_t blinkPeriod;
    TimeEvent ledTimer;
} Led;

enum LedEventSignals
{
    LED_OFF_SIG = USER_SIG,
    LED_ON_SIG,
    ARM_BLINK_SIG,
    DISARM_BLINK_SIG,
    BLINK_TIMER_EXPIRED_SIG,
};

void Led_ctor(Led * const me);

#endif