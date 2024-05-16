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

static void Led_off(void)
{
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
}

static void Led_on(void)
{
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 1));
}

static State Led_initial(Led * const me, Event const * const e)
{
    return TRAN(Led_idle);
}

static State Led_idle(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        Led_off();
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        break;
    
    case EV_BUTTON_PRESSED:
        TRAN(Led_blink);
        break;
    
    default:
        status = IGNORED_STATUS;
        break;
    }
}

static State Led_blink(Led * const me, Event const * const e)
{

}



static void Led_dispatch(Led * const me, Event const * const e)
{
    if (e->sig == INIT_SIG)
    {
        Led_off();
        me->state = OFF_STATE;
    }

    switch (me->state)
    {
    case OFF_STATE:
        switch (e->sig)
        {
        case BLINK_TIMER_EXPIRED_SIG:
            Led_on();
            me->state = ON_STATE;
            break;

        case EV_BUTTON_PRESSED:
            if (me->blinkPeriod <= 300)
            {
                me->blinkPeriod +=100;
            } else {
                me->blinkPeriod = 100;
            }
            //DEBUG
            ESP_LOGI("LED", "period: %d", me->blinkPeriod);
            //DEBUG
            TimeEvent_change_period(&me->ledTimer, (TickType_t)(me->blinkPeriod/portTICK_PERIOD_MS));
            break;
        
        case EV_BUTTON_HOLD:
            TimeEvent_disarm(&me->ledTimer);
            break;
        
        case EV_BUTTON_DOUBLE_PRESS:
            break;

        default:
            break;
        }
        break;

    case ON_STATE:
        switch (e->sig)
        {
        case BLINK_TIMER_EXPIRED_SIG:
            Led_off();
            me->state = OFF_STATE;
            break;
        
        case EV_BUTTON_PRESSED:
            if (me->blinkPeriod <= 300)
            {
                me->blinkPeriod += 100;
            } else {
                me->blinkPeriod = 100;
            }
            //DEBUG
            ESP_LOGI("LED", "period: %d", me->blinkPeriod);
            //DEBUG
            TimeEvent_change_period(&me->ledTimer, (TickType_t)(me->blinkPeriod/portTICK_PERIOD_MS));
            break;
        
        case EV_BUTTON_HOLD:
            TimeEvent_disarm(&me->ledTimer);
            break;
        
        case EV_BUTTON_DOUBLE_PRESS:
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

void Led_ctor(Led * const me)
{
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    me->blinkPeriod = 100;
    Active_ctor(&me->super, (DispatchHandler)&Led_dispatch);
    TimeEvent_ctor(&me->ledTimer, "LED timer", (TickType_t)(me->blinkPeriod/portTICK_PERIOD_MS), pdTRUE, BLINK_TIMER_EXPIRED_SIG, &me->super);
}