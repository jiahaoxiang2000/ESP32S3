#ifndef __UART_M_H_
#define __UART_M_H_


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/uart_select.h"
#include "driver/gpio.h"



#define USART_UX            UART_NUM_1
#define USART_TX_GPIO_PIN   GPIO_NUM_17
#define USART_RX_GPIO_PIN   GPIO_NUM_18


#define RX_BUF_SIZE         1024    


void usart_init(uint32_t baudrate); 

#endif // __UART_M_H_
