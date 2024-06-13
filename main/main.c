#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "led.h"
#include "uart.h"
#include "lcd.h"
#include "dht11.h"

i2c_obj_t i2c0_master;

void app_main(void)
{
    uint8_t err;
    uint8_t t = 0;
    uint8_t temperature;
    uint8_t humidity;
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

    err = dht11_init();

    if (err != 0)
    {
        while (1)
        {
            lcd_show_string(30, 50, 200, 16, 16, "DHT11 Error", RED);
            vTaskDelay(200);
            lcd_fill(30, 50, 239, 130 + 16, WHITE);
            vTaskDelay(200);
        }
    }

    lcd_show_string(30, 50, 200, 16, 16, "DHT11 OK", RED);
    lcd_show_string(30, 70, 200, 16, 16, "Temp:  C", BLUE);
    lcd_show_string(30, 90, 200, 16, 16, "Humi:  %", BLUE);

    while (1)
    {
        if (t % 10 == 0)                                            
        {
            dht11_read_data(&temperature, &humidity);               /* 读取温湿度值 */
            lcd_show_num(30 + 40, 70, temperature, 2, 16, BLUE);   /* 显示温度 */
            lcd_show_num(30 + 40, 90, humidity, 2, 16, BLUE);      /* 显示湿度 */
        }

        vTaskDelay(10);
        t++;
        if (t == 50)
        {
            t = 0;
            uart_write_bytes(USART_UX, (const char *)data, strlen((const char *)data));
            LED_TOGGLE(); /* LED闪烁 */
        }
    }
}