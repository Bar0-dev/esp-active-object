#ifndef _LED_AO_H_
#define _LED_AO_H_
#include "esp_ao.h"

#define LED_PIN 27
#define BLINK_PERIOD 200

typedef struct
{
    Active super;

    bool state;
    TimeEvent *ledTimer;
} Led;

enum EventSignals
{
    LED_OFF_SIG = USER_SIG,
    LED_ON_SIG,
    ARM_BLINK_SIG,
    DISARM_BLINK_SIG,
    BLINK_TIMER_EXPIRED_SIG,
};

void Led_ctor(Led * const me);

#endif