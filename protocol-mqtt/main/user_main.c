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

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

#include "myiic.h"
#include "oled_iic.h"

#define EXAMPLE_WIFI_SSID "CMCC-BBIG-YD"
#define EXAMPLE_WIFI_PASS "LIN888+++"

#define SEND_KEY_PIN    14
#define SEND_KEY_PIN_SEL  (1ULL<<SEND_KEY_PIN)

static const char *TAG = "mqtt test";

static char MQTT_STATUS_BUFF[64];

static EventGroupHandle_t wifi_event_group;     //基于FreeRTOS的wifi事件组，方便处理wifi事件并回调函数
static EventGroupHandle_t mqtt_event_group;

const int CONNECTED_BIT         = BIT0;         //规定BIT0为连接标志位
const int MQTT_STATUS_BIT       = BIT1;

static int MQTT_ONLINE = 0;

static esp_mqtt_client_handle_t client;         //MQTT客户端

//WIFI连接事件回调处理函数
static esp_err_t event_handler(void *ctx, system_event_t *event)    
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

//MQTT事件回调函数
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)  
{
    esp_mqtt_client_handle_t client = event->client;

    memset(MQTT_STATUS_BUFF, 0 ,sizeof(MQTT_STATUS_BUFF));

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            MQTT_ONLINE = 1;
            esp_mqtt_client_subscribe(client, "/esp8266/set", 0); //默认订阅  
            strcpy(MQTT_STATUS_BUFF, "MQTT CONNECTED");

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            MQTT_ONLINE = 0;
            strcpy(MQTT_STATUS_BUFF, "MQTT DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            strcpy(MQTT_STATUS_BUFF, "MQTT SUBSCRIBED");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            strcpy(MQTT_STATUS_BUFF, "MQTT UNSUBSCRIBED");
            break;
        case MQTT_EVENT_PUBLISHED:
            strcpy(MQTT_STATUS_BUFF, "MQTT PUBLISHED");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            strcpy(MQTT_STATUS_BUFF, "MQTT RECEIVE");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            strcpy(MQTT_STATUS_BUFF, "MQTT ERROR");
            break;
    }
    xEventGroupSetBits(mqtt_event_group, MQTT_STATUS_BIT);
    return ESP_OK;
}

static void key_init(void)
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_DISABLE;  //失能中断
    io_conf.mode = GPIO_MODE_INPUT;         //输入模式
    io_conf.pin_bit_mask = SEND_KEY_PIN_SEL;//指定GPIO14作为按键引脚
    io_conf.pull_down_en = 1;               //下拉
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf); 

}

static void wifi_init(void)           //连接wifi函数
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
    
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);   //等待wifi连接成功
}

static void send_hello(void *arg)
{
    key_init();     //按键初始化

    while(1)
    {
        if(gpio_get_level(SEND_KEY_PIN) && MQTT_ONLINE == 1){
            esp_mqtt_client_publish(client, "/esp8266/post", "hello", 0, 1, 0);
        }
        vTaskDelay(50 / portTICK_RATE_MS);  //给个短延时，避免看门狗超时复位
    }
}


static void showMSG_task(void *arg)
{
    OLED_Init();    //OLED初始化
    OLED_Clear();   //OLED清屏幕

    OLED_ShowString(24, 0, (uint8_t *)"system init", 8);

    while(1)
    {
        if(MQTT_ONLINE)
            OLED_ShowString(24, 0, (uint8_t *)"MQTT ONLINE", 8);
        else
            OLED_ShowString(24, 0, (uint8_t *)"MQTT OFFLINE", 8);
        
        xEventGroupWaitBits(mqtt_event_group, MQTT_STATUS_BIT, false, true, portMAX_DELAY);
        
        OLED_ShowString(0, 2, (uint8_t *)MQTT_STATUS_BUFF, 16); 
        
        vTaskDelay(1000 / portTICK_RATE_MS);

        xEventGroupClearBits(mqtt_event_group, MQTT_STATUS_BIT);
        OLED_Clear();   //OLED清屏幕
        
    }

}


static void mqtt_app_start(void)
{
    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .host = "192.168.1.20",
        .port = 1883,
        .username = "esp8266",
        .password = "123456",
        .event_handle = mqtt_event_handler,
    };
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
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

    wifi_init();    //连接wifi

    xTaskCreate(showMSG_task, "showMSG_task", 2048, NULL, 10, NULL);
    xTaskCreate(send_hello, "send_hello", 1024, NULL, 10, NULL);

    mqtt_app_start();
    
}



