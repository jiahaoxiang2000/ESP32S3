#include <stdio.h>
#include "led.h"

void led_init(void)
{
    gpio_config_t gpio_init_struct = {0};

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;        
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;        
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;     
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE; 
    gpio_init_struct.pin_bit_mask = 1ull << LED_GPIO_PIN;  
    gpio_config(&gpio_init_struct);                        

    LED(1); /* 关闭LED */
}