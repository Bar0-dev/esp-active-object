#include <stdio.h>
#include "led_ao.h"

void app_main(void)
{
    led_ao_init();
    led_off();
}
