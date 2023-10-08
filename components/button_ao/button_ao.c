#include "driver/gpio.h"
#include "esp_log.h"

#include "events_broker.h"
#include "button_ao.h"

static const gpio_config_t gpioConfig =
    {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
};

static bool prevState = false;

static void Button_dispatch(Button *const me, Event const *const e)
{
    if (e->sig == INIT_SIG)
    {
        bool currState = (bool)gpio_get_level(BUTTON_PIN);
        prevState = currState;
        TimeEvent_arm(&me->pollTimer);
        if (currState)
        {
            me->state = RELEASED;
        }
        else
        {
            me->state = PRESSED;
        }
        Active_post(&me->super, &(Event){ ENTRY_SIG });
    }

    /**
     * POLLING
     */
    if (e->sig == BUTTON_POLL_SIG)
    {
        bool currState = (bool)gpio_get_level(BUTTON_PIN);
        if (currState != prevState)
        {
            TimeEvent_arm(&me->debounceTimer);
            TimeEvent_disarm(&me->pollTimer);
        }
    }

    if (e->sig == BUTTON_DEBOUNCED_SIG)
    {
        bool currState = (bool)gpio_get_level(BUTTON_PIN);
        if (currState != prevState)
        {
            Active_post(&me->super, &(Event){BUTTON_STATE_CHANGED_SIG});
            prevState = currState;
        }
        TimeEvent_arm(&me->pollTimer);
    }

    /**
     * BUTTON STATE MACHINE
     */

    switch (me->state)
    {
    case RELEASED:
        switch (e->sig)
        {
        case ENTRY_SIG:
            Active_post(AO_Broker, &(Event){ EV_BUTTON_RELEASED });
            break;

        case BUTTON_STATE_CHANGED_SIG:
            me->state = PRESSED;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;

        default:
            break;
        }
        break;

    case PRESSED:
        switch (e->sig)
        {
        case ENTRY_SIG:
            TimeEvent_arm(&me->holdTimer);
            Active_post(AO_Broker, &(Event){ EV_BUTTON_PRESSED });
            break;

        case BUTTON_STATE_CHANGED_SIG:
            me->state = PRE_DOUBLE_PRESSED;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;
        
        case BUTTON_HOLD_SIG:
            me->state = HOLD;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;
        
        default:
            break;
        }
        break;

    case PRE_DOUBLE_PRESSED:
        switch (e->sig)
        {
        case ENTRY_SIG:
            TimeEvent_disarm(&me->holdTimer);
            TimeEvent_arm(&me->doublePressTimer);
            break;
        
        case BUTTON_STATE_CHANGED_SIG:
            me->state = DOUBLE_PRESSED;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;
        
        case BUTTON_DOUBLE_PRESS_SIG:
            me->state = RELEASED;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;

        case BUTTON_HOLD_SIG:
            me->state = HOLD;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;

        default:
            break;
        }
        break;

    case HOLD:
        switch (e->sig)
        {
        case ENTRY_SIG:
            Active_post(AO_Broker, &(Event){ EV_BUTTON_HOLD });
            break;
        
        case BUTTON_STATE_CHANGED_SIG:
            me->state = RELEASED;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;

        default:
            break;
        }
        break;

    case DOUBLE_PRESSED:
        switch (e->sig)
        {
        case ENTRY_SIG:
            TimeEvent_disarm(&me->doublePressTimer);
            Active_post(AO_Broker, &(Event){ EV_BUTTON_DOUBLE_PRESS });
            break;
        
        case BUTTON_STATE_CHANGED_SIG:
            me->state = RELEASED;
            Active_post(&me->super, &(Event){ ENTRY_SIG });
            break;

        default:
            break;
        }
        break;

    default:
        assert(0);
        break;
    }
}

void Button_ctor(Button *const me)
{
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    Active_ctor(&me->super, (DispatchHandler)&Button_dispatch);
    TimeEvent_ctor(&me->debounceTimer, "Debouce timer", (TickType_t)(DEBOUNCE_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_DEBOUNCED_SIG, &me->super);
    TimeEvent_ctor(&me->pollTimer, "Poll timer", (TickType_t)(POLL_TIME / portTICK_PERIOD_MS), pdTRUE, BUTTON_POLL_SIG, &me->super);
    TimeEvent_ctor(&me->holdTimer, "Hold timer", (TickType_t)(HOLD_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_HOLD_SIG, &me->super);
    TimeEvent_ctor(&me->doublePressTimer, "Double press timer", (TickType_t)(DOUBLE_PRESS_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_DOUBLE_PRESS_SIG, &me->super);
}