#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "led.h"
#include "uart.h"
#include "lcd.h"
#include "dht11.h"

i2c_obj_t i2c0_master;

static const uint8_t sbox[16] = {
    0x00, 0x0f, 0x0e, 0x05,
    0x0d, 0x03, 0x06, 0x0c,
    0x0b, 0x09, 0x0a, 0x08,
    0x07, 0x04, 0x02, 0x01};

static uint8_t rPx[16] = {
    0, 7, 10, 13, 4, 11, 14, 1, 8, 15, 2, 5, 12, 3, 6, 9};

void Subcells(uint8_t *s)
{
    for (int i = 0; i < 16; i++)
        s[i] = sbox[s[i]];
}

void Exchang(uint8_t *s)
{
    int i;
    uint8_t g[16];
    memset(g, 0, sizeof(g));
    for (i = 0; i < 16; i++)
        g[i] = s[rPx[i]];
    memcpy(s, g, sizeof(g));
}
uint8_t FileMutil(uint8_t x)
{
    x <<= 1;
    if (x & 0xf0)
        x = (x ^ 0x03);
    return x & 0x0f;
}
void Cconfusion(uint8_t *s)
{
    int i;
    uint8_t te[16];
    for (size_t i = 0; i < 16; i++)
    {
        te[i] = 0;
    }

    for (i = 0; i < 4; i++)
    {
        te[4 * i] = s[4 * i + 2] ^ s[4 * i + 1] ^ s[4 * i + 3];
        te[4 * i + 1] = s[4 * i + 3] ^ s[4 * i] ^ s[4 * i + 2];
        te[4 * i + 2] = s[4 * i] ^ s[4 * i + 1] ^ s[4 * i + 3];
        te[4 * i + 3] = s[4 * i + 1] ^ s[4 * i] ^ s[4 * i + 2];
    }
    for (size_t i = 0; i < 16; i++)
    {
        s[i] = te[i];
    }
}
void addkeys(uint8_t *k, uint8_t *s)
{
    for (int i = 0; i < 16; i++)
        s[i] ^= k[i];
}
// void Updatekeys(uint8_t *k, int j) // 80 bit
// {
//     int i;
//     uint8_t tem[20];
//     k[19] = sbox[k[19]];
//     k[10] = (k[10] ^ (j >> 1)) & 0x0f;
//     k[11] = (k[11] ^ (j << 3)) & 0x0f;
//     for (i = 0; i < 20; i++)
//         tem[i] = k[i];
//     for (i = 0; i < 10; i++)
//         k[i] = ((tem[(i + 2) % 10] << 3) | (tem[(i + 3) % 10] >> 1)) & 0x0f;
//     for (i = 0; i < 20; i++)
//         tem[i] = k[i];

//     for (i = 0; i < 10; i++)
//         k[i + 10] = tem[i];
//     for (i = 10; i < 20; i++)
//         k[i - 10] = tem[i];
// }

void Updatekeys(uint8_t *k, int j) // 128 bit
{
    int i;
    uint8_t tem[32], aa, bb;
    k[31] = sbox[k[31]];
    k[30] = sbox[k[30]];
    k[16] = (k[16] ^ (j >> 1)) & 0x0f;
    k[17] = (k[17] ^ (j << 3)) & 0x0f;
    for (i = 0; i < 32; i++)
        tem[i] = k[i];
    for (i = 0; i < 16; i++)
        k[i] = ((tem[(i + 1) % 16] << 3) | (tem[(i + 2) % 16] >> 1)) & 0x0f;
    for (i = 0; i < 32; i++)
        tem[i] = k[i];

    for (i = 0; i < 16; i++)
        k[i + 16] = tem[i];
    for (i = 16; i < 32; i++)
        k[i - 16] = tem[i];
}

void Encrypt(uint8_t *s, uint8_t *k)
{
    int i;
    for (i = 0; i < 28; i++)
    {
        Updatekeys(k, i + 1);
        addkeys(k, s);
        Subcells(s);
        Exchang(s);
        Cconfusion(s);
    }
    Updatekeys(k, i + 1);
    addkeys(k, s);
}

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
            dht11_read_data(&temperature, &humidity);            /* 读取温湿度值 */
            lcd_show_num(30 + 40, 70, temperature, 2, 16, BLUE); /* 显示温度 */
            lcd_show_num(30 + 40, 90, humidity, 2, 16, BLUE);    /* 显示湿度 */

            char plaintext[40]; // Buffer for temperature in hex, 4 chars + null terminator
            char pt[17];        // Assuming these functions exist

            // Assuming these functions exist
            sprintf(pt, "000000000000%X%X", temperature, humidity);            //
            sprintf(plaintext, "pt: 000000000000%X%X", temperature, humidity); //

            // Assuming lcd_show_string exists and can be used similarly to lcd_show_num
            lcd_show_string(30, 110, 200, 16, 16, plaintext, BLUE); // Display temperature in hex

            uint8_t key[32] = {6, 3, 5, 8, 2, 7, 4, 1, 6, 3, 5, 8, 2, 7, 4, 1, 6, 3, 5, 8, 2, 7, 4, 1, 6, 3, 5, 8, 2, 7, 4, 1};
            Encrypt((uint8_t *)pt, (uint8_t *)key);

            sprintf(plaintext, "et: %X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X", pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], pt[6], pt[7], pt[8], pt[9], pt[10], pt[11], pt[12], pt[13], pt[14], pt[15]); //

            lcd_show_string(30, 130, 200, 16, 16, plaintext, BLUE); //

            // lcd_show_string(30, 130, 200, 16, 16, humiHex, BLUE); // Display humidity in hex
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