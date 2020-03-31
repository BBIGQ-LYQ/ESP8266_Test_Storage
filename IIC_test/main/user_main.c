#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "myiic.h"
#include "oled_iic.h"


static const char *TAG = "iic_test";

static void i2c_task(void *arg)
{
    uint8_t pBuff[32] = "hello 8266 MUA";
    OLED_Init();
    OLED_Clear();
    while(1)
    {
        OLED_ShowString(8,2,pBuff,16); 
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


void app_main(void)
{

    printf("SDK version:%s\n", esp_get_idf_version());

    xTaskCreate(i2c_task, "i2c_task", 2048, NULL, 10, NULL);

}



