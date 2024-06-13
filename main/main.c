#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_psram.h"
#include "esp_flash.h"
#include "led.h"

void app_main(void)
{
    led_init();
    printf("led init\r\n");
    while (1)
    {
        LED(1);
        vTaskDelay(1000);
        LED(0);
        vTaskDelay(1000);
    }
}