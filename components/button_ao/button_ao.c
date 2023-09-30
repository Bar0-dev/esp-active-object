#include "driver/gpio.h"
#include "esp_log.h"

#include "button_ao.h"


static const gpio_config_t gpioConfig = 
{
    .pin_bit_mask = (1ULL<<BUTTON_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

static void Button_pressed_handler(void)
{
    ESP_LOGI("Button", "Pressed");
}

static void Button_released_handler(void)
{
    ESP_LOGI("Button", "Released");
}
static void Button_dispatch(Button * const me, Event const * const e)
{
    static volatile bool currState;

    switch (e->sig)
    {
    case INIT_SIG:
        me->state = true;
        Active_post(&me->super, &(Event){ BUTTON_POLL_SIG });
        break;

    case BUTTON_POLL_SIG:
        currState = (bool)gpio_get_level(BUTTON_PIN);
        if(me->state != currState)
        {
            TimeEvent_arm(&me->debounceTimer);
        }
        break;

    case BUTTON_DEBOUNCED_SIG:
        currState = (bool)gpio_get_level(BUTTON_PIN);
        if(me->state != currState)
        {
            if (currState)
            {
                Active_post(&me->super, &(Event){ BUTTON_RELEASED_SIG });
            } else {
                Active_post(&me->super, &(Event){ BUTTON_PRESSED_SIG });
            }
            me->state = currState;
        } else {
            Active_post(&me->super, &(Event){ BUTTON_POLL_SIG });
        }
        break;
    
    case BUTTON_PRESSED_SIG:
        Button_pressed_handler();
        Active_post(&me->super, &(Event){ BUTTON_POLL_SIG });
        break;
    
    case BUTTON_RELEASED_SIG:
        Button_released_handler();
        Active_post(&me->super, &(Event){ BUTTON_POLL_SIG });
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
}
