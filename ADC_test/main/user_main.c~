#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"

static const char *TAG = "adc example";

static void adc_task()
{
    int x;
    uint16_t adc_data[100];

    while (1) {
    
        if (ESP_OK == adc_read(&adc_data[0])) {
            ESP_LOGI(TAG, "adc read: %d\r\n", adc_data[0]);
        }
        //读100个数值
        /*
        if (ESP_OK == adc_read_fast(adc_data, 100)) {
            for (x = 0; x < 100; x++) {
                printf("%d -> %d\n", x, adc_data[x]);
            }
        }
        */
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main()
{
    
    adc_config_t adc_config;                            // ADC init结构体
    adc_config.mode = ADC_READ_TOUT_MODE;               //设置读取外部电压
    adc_config.clk_div = 8;                             // ADC sample collection clock = 80MHz/clk_div = 10MHz
    ESP_ERROR_CHECK(adc_init(&adc_config));             //ADC初始化

    xTaskCreate(adc_task, "adc_task", 1024, NULL, 5, NULL);//创建ADC任务
}

