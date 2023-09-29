#include <stdio.h>
#include "led_ao.h"
#include "driver/gpio.h"

static const gpio_config_t gpioConfig = 
{
    .pin_bit_mask = (1ULL<<LED_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

void led_ao_init(void)
{
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
}

void led_on(void){
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 1));
}

void led_off(void){
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
}