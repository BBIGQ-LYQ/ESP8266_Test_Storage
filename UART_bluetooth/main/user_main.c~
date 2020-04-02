#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"

static const char *TAG = "bluetooth test";


static void ble_task(void *arg)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,                      //蓝牙波特率
        .data_bits = UART_DATA_8_BITS,          //8位数据位
        .parity    = UART_PARITY_DISABLE,       //无奇偶校验
        .stop_bits = UART_STOP_BITS_1,          //1位停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE   //无硬件流
    };
    
    uart_param_config(UART_NUM_0, &uart_config);                //配置初始化
    uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL);  //安装串口驱动
    
    uint8_t *pBuff = (uint8_t *) malloc(1024);
    
     while (1) {
        memset(pBuff, 0, 1024);
        int len = uart_read_bytes(UART_NUM_0, pBuff, 1024, 20 / portTICK_RATE_MS);
        if(len > 0)
            ESP_LOGI(TAG, "pBuff : %s\n", pBuff);
    }

}


void app_main(void)
{    
    xTaskCreate(ble_task, "uart_ble_task", 1024, NULL, 10, NULL);
}



