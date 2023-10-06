#include "led_ao.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

static const gpio_config_t gpioConfig = 
{
    .pin_bit_mask = (1ULL<<LED_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

static void Led_dispatch(Led * const me, Event const * const e)
{
    switch (e->sig)
    {
    case INIT_SIG:
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
        me->state = false;
        break;

    case EV_BUTTON_PRESSED:
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 1));
        me->state = true;
        TimeEvent_arm(&me->ledTimer);
        break;
    
    case EV_BUTTON_RELEASED:
        TimeEvent_disarm(&me->ledTimer);
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
        break;

    case BLINK_TIMER_EXPIRED_SIG:
        me->state = !me->state;
        ESP_ERROR_CHECK(gpio_set_level(LED_PIN, me->state));
        break;

    default:
        break;
    }
}

void Led_ctor(Led * const me)
{
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    me->blinkPeriod = BLINK_PERIOD_DEFAULT;
    Active_ctor(&me->super, (DispatchHandler)&Led_dispatch);
    TimeEvent_ctor(&me->ledTimer, "LED timer", (TickType_t)(me->blinkPeriod/portTICK_PERIOD_MS), pdTRUE, BLINK_TIMER_EXPIRED_SIG, &me->super);
}