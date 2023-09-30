#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "led_ao.h"
#include "button_ao.h"

static Led led;
static Active *AO_Led = &led.super;
static Event const evt1 = { LED_ON_SIG };
static Event const evt2 = { ARM_BLINK_SIG };

// static Button button;
// static Active *AO_Button = &button.super;

void app_main(void)
{
    Led_ctor(&led);
    Active_start(AO_Led, "LED thread", 2048, 1, tskNO_AFFINITY, 10);
    Active_post(AO_Led, &evt1);
    Active_post(AO_Led, &evt2);
    // Button_ctor(&button);
    // Active_start(AO_Button, "Button thread", 1024, 2, tskNO_AFFINITY, 10);
}