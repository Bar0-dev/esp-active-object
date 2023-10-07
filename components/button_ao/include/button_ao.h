#ifndef BUTTON_AO_H
#define BUTTON_AO_H

#include "esp_ao.h"

#define BUTTON_PIN 25
#define DEBOUNCE_TIME 15
#define POLL_TIME 50
#define HOLD_TIME 1000
#define DOUBLE_PRESS_TIME 300

typedef struct
{
    Active super;
    bool state;

    TimeEvent debounceTimer;
    TimeEvent pollTimer;
    TimeEvent holdTimer;
    TimeEvent doublePressTimer;
} Button;

enum ButtonEventSignals
{
    BUTTON_POLL_SIG = USER_SIG,
    BUTTON_DEBOUNCED_SIG,
    BUTTON_HOLD_SIG,
    BUTTON_DOUBLE_PRESS_SIG,
};

void Button_ctor(Button * const me);

/**
 * Active objects
*/
extern Active *AO_Broker;

#endif