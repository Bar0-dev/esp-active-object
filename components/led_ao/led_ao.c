#include <stdio.h>
#include "led_ao.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static void Led_dispatch(Led * const me, Event const * const e)
{
    switch (e->sig)
    {
    case INIT_SIG:
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
        break;

    case LED_ON_SIG:
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 1));
        break;

    case LED_OFF_SIG:
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
        break;

    case ARM_BLINK_SIG:
        TimeEvent_ctor(me->ledTimer, "LED timer", BLINK_TIMER_EXPIRED_SIG, (Active*) me);
        TimeEvent_arm(me->ledTimer, BLINK_PERIOD, pdTRUE);
    
    case DISARM_BLINK_SIG:
        TimeEvent_disarm(me->ledTimer);

    case BLINK_TIMER_EXPIRED_SIG:

    default:
        break;
    }
}

void Led_ctor(Led * const me)
{
    static const gpio_config_t gpioConfig = 
    {
        .pin_bit_mask = (1ULL<<LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    Active_ctor(&me->super, (DispatchHandler)&Led_dispatch);
}