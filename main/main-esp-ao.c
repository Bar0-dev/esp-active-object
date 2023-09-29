#include <stdio.h>
#include "led_ao.h"

void app_main(void)
{
    Led led;
    Led_ctor(&led);
    Event led_e = { LED_ON_SIG };
    Active_start((Active*) &led)
    Active_post((Active*) &led, &led_e);
}
