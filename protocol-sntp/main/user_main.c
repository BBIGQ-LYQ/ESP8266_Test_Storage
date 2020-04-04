#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "lwip/apps/sntp.h"

#include "myiic.h"
#include "oled_iic.h"

#define EXAMPLE_WIFI_SSID "你的WIFI名"
#define EXAMPLE_WIFI_PASS "你的WIFI密码"

static const char *TAG = "sntp_test&show_time";

static EventGroupHandle_t wifi_event_group;     //基于FreeRTOS的wifi事件组，方便处理wifi事件并回调函数
const int CONNECTED_BIT = BIT0;                 //规定BIT0为连接标志位

static esp_err_t event_handler(void *ctx, system_event_t *event)    //WIFI连接事件回调处理函数
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
            if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
                /*Switch to 802.11 bgn mode */
                esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
            }
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;

        default:
            break;
    }

    return ESP_OK;
}

static void initialise_wifi(void)           //连接wifi函数
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "WIFI SSID： %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void initialize_sntp(void)               //初始化SNTP
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);    //设置为单播模式    
    sntp_setservername(0, "pool.ntp.org");      //设置SNTP服务器
    sntp_init();                                //初始化，并向SNTP服务器发出校时请求
}

static void obtain_time(void)
{
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);    //等待WIFI连接成功
    initialize_sntp();      //wifi连接成功后进行SNTP校时                         

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    
    //8266上电没校时的话，时钟是1970年开始的，这里的while设置了20s超时处理
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) { 
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);     
        time(&now);
        localtime_r(&now, &timeinfo);       //获取系统时间，如果系统时间被成功修改后，今年时2020年，timeinfo.tm_year应该为120 > 116
    }
}


static void showtime_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    
    char strftime_datebuf[32];  //保存日期
    char strftime_timebuf[32];  //保存时间
    
    OLED_Init();    //OLED初始化
    OLED_Clear();   //OLED清屏幕
    
    time(&now);     
    localtime_r(&now, &timeinfo);           //获取本地时间
    if (timeinfo.tm_year < (2016 - 1900)) { //判断是否为修改过
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();      //获取时间
    }

    setenv("TZ", "CST-8", 1);           //设置为中国区时间
    tzset();                            //设置时间环境变量
    
    while(1)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        
        if (timeinfo.tm_year < (2016 - 1900)) {
            ESP_LOGE(TAG, "The current date/time error");
        } else {
            strftime(strftime_datebuf, sizeof(strftime_datebuf), "%F", &timeinfo);//取出日期/年月日
            strftime(strftime_timebuf, sizeof(strftime_timebuf), "%T", &timeinfo);//取出时间/时分秒
            //ESP_LOGI(TAG, "The current date in Shanghai is: %s", strftime_datebuf);
            //ESP_LOGI(TAG, "The current time in Shanghai is: %s", strftime_timebuf);
            OLED_ShowString(24, 0, (uint8_t *)strftime_datebuf, 16); 
            OLED_ShowString(32, 2, (uint8_t *)strftime_timebuf, 16); 
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
        
    }
}

void app_main(void)
{

    //初始化 NVS
    esp_err_t ret = nvs_flash_init();   //初始化默认的NVS分区
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    initialise_wifi();  //连接wifi

    // SNTP服务使用LwIP，需要分配大的栈空间。
    xTaskCreate(showtime_task, "sntp_gettime_task", 2048, NULL, 10, NULL);

}



