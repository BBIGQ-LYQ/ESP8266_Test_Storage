#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#include "driver/pwm.h"


#define PWM_PERIOD      1000

static const char *TAG = "pwm example";
const uint32_t PWM_OUT_IO_NUM = 5;
uint32_t duties = 900;
int16_t phase = 0;
void app_main()
{
    pwm_init(PWM_PERIOD, &duties, 1, &PWM_OUT_IO_NUM);       //PWM的初始化，周期1ms->1KHz,各通道工作周期500us，1条通道，GPIO5
    pwm_set_phases(&phase);                                  //初始相位0
    pwm_start();                                             //开始PWM输出
    int16_t count = 0;

    while (1) {
        if (count == 20) {
            pwm_stop(0x1);                     //此处停止PWM并将引脚变成高电平
            ESP_LOGI(TAG, "PWM stop\n");
        } else if (count == 30) {
            pwm_set_duty(0, 500);              //将占空比改成50%
            pwm_start();                       //使修改占空比生效，需要再次开启
            ESP_LOGI(TAG, "PWM re-start\n");
            count = 0;
        }

        count++;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    
}

