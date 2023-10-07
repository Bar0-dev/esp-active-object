#include "driver/gpio.h"
#include "esp_log.h"

#include "events_broker.h"
#include "button_ao.h"


static const gpio_config_t gpioConfig = 
{
    .pin_bit_mask = (1ULL<<BUTTON_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

static void Button_dispatch(Button * const me, Event const * const e)
{
    bool currState = (bool)gpio_get_level(BUTTON_PIN);

    switch (e->sig)
    {
    case INIT_SIG:
        me->state = currState;
        TimeEvent_arm(&me->pollTimer);
        break;

    case BUTTON_POLL_SIG:
        if (me->state != currState)
        {
            TimeEvent_arm(&me->debounceTimer);
        }
        break;

    case BUTTON_DEBOUNCED_SIG:
        if (me->state != currState)
        {
            if (currState)
            {
                Active_post(AO_Broker, &(Event){ EV_BUTTON_RELEASED });
                TimeEvent_arm(&me->doublePressTimer);
                TimeEvent_disarm(&me->holdTimer);
            } else {
                Active_post(AO_Broker, &(Event){ EV_BUTTON_PRESSED });
                TimeEvent_arm(&me->holdTimer);
                me->pressCount++;
            }
            me->state = currState;
        }
        break;
    
    case BUTTON_HOLD_SIG:
        if (currState == false)
        {
            Active_post(AO_Broker, &(Event){ EV_BUTTON_HOLD });
        }
        me->state = currState;
        break;

    case BUTTON_DOUBLE_PRESS_SIG:
        if (me->pressCount >= 2)
        {
            Active_post(AO_Broker, &(Event){ EV_BUTTON_DOUBLE_PRESS });
        }
        me->pressCount = 0;
        break;

    default:
        break;
    }
}

void Button_ctor(Button * const me)
{
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    Active_ctor(&me->super, (DispatchHandler)&Button_dispatch);
    TimeEvent_ctor(&me->debounceTimer, "Debouce timer", (TickType_t)(DEBOUNCE_TIME/portTICK_PERIOD_MS), pdFALSE, BUTTON_DEBOUNCED_SIG, &me->super);
    TimeEvent_ctor(&me->pollTimer, "Poll timer", (TickType_t)(POLL_TIME/portTICK_PERIOD_MS), pdTRUE, BUTTON_POLL_SIG, &me->super);
    TimeEvent_ctor(&me->holdTimer, "Hold timer", (TickType_t)(HOLD_TIME/portTICK_PERIOD_MS), pdFALSE, BUTTON_HOLD_SIG, &me->super);
    TimeEvent_ctor(&me->doublePressTimer, "Double press timer", (TickType_t)(DOUBLE_PRESS_TIME/portTICK_PERIOD_MS), pdFALSE, BUTTON_DOUBLE_PRESS_SIG, &me->super);
}