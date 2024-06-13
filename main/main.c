#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "led.h"
#include "uart.h"
#include "lcd.h"

i2c_obj_t i2c0_master;

void app_main(void)
{
    esp_err_t ret;
    unsigned char data[RX_BUF_SIZE] = "Hello World!\r\n";
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    // init peripheral
    led_init();
    usart_init(115200);
    i2c0_master = iic_init(I2C_NUM_0); 
    spi2_init();                        
    xl9555_init(i2c0_master);          
    lcd_init();


    while (1)
    {
        lcd_show_string(10, 40, 240, 32, 32, "ESP32", BLACK);
        lcd_show_num(10, 72, 240, 2, 32, BLACK);

        uart_write_bytes(USART_UX, (const char *)data, strlen((const char *)data));
        LED_TOGGLE();
        vTaskDelay(500);
        
    }
}