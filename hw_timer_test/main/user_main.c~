#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/hw_timer.h"

static const char *TAG = "hw_timer_example";

#define TEST_ONE_SHOT    false        
#define TEST_RELOAD      true         

#define GPIO_OUTPUT_IO_0    12
#define GPIO_OUTPUT_PIN_SEL  (1ULL << GPIO_OUTPUT_IO_0)

void hw_timer_test(void *arg)
{
    static int state = 0;

    gpio_set_level(GPIO_OUTPUT_IO_0, (state ++) % 2);
}

void app_main(void)
{
    //GPIO初始化
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    //初始化硬件定时器
    hw_timer_init(hw_timer_test, NULL);
    //设置输出频率为500Hz
    hw_timer_alarm_us(1000, TEST_RELOAD);
    
    while(1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "Testting");
    }
    
}




