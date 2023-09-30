#ifndef BUTTON_AO_H
#define BUTTON_AO_H

#include "esp_ao.h"

#define BUTTON_PIN 25
#define DEBOUNCE_TIME 1000

typedef struct
{
    Active super;
    bool state;

    TimeEvent debounceTimer;
} Button;

enum ButtonEventSignals
{
    BUTTON_POLL_SIG = USER_SIG,
    BUTTON_DEBOUNCED_SIG,
    BUTTON_PRESSED_SIG,
    BUTTON_RELEASED_SIG,
};

void Button_ctor(Button * const me);

#endif