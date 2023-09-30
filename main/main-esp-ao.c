#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "led_ao.h"
#include "button_ao.h"

static Led led;
Active *AO_Led = &led.super;
static Button button;
Active *AO_Button = &button.super;

void app_main(void)
{
    Led_ctor(&led);
    Active_start(AO_Led, "LED thread", 2048, 10, tskNO_AFFINITY, 10);

    Button_ctor(&button);
    Active_start(AO_Button, "Button thread", 2048, 1, tskNO_AFFINITY, 10);
}