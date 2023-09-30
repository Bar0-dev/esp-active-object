#include <stdio.h>
#include "led_ao.h"
#include "freertos/FreeRTOS.h"

#include "esp_log.h"

static Led led;
static Active *AO_Led = &led.super;
static Event const evt1 = { LED_ON_SIG };
static Event const evt2 = { ARM_BLINK_SIG };


void app_main(void)
{
    Led_ctor(&led);
    Active_start(AO_Led, "LED thread", 2048, 1, tskNO_AFFINITY, 10);
    Active_post(AO_Led, &evt1);
    Active_post(AO_Led, &evt2);
}