#include <stdio.h>

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>

#include "esp_system.h"

#include "BBIGQ_TCP.h"

#define EXAMPLE_WIFI_SSID "CMCC-BBIG-YD"
#define EXAMPLE_WIFI_PASS "LIN888+++"

static EventGroupHandle_t wifi_event_group;

const int CONNECTED_BIT = BIT0;

#define WEB_SERVER "47.107.120.234"
#define WEB_PORT 80
#define WEB_URL "/api.php?key=free&appid=0&msg=天气广州"

#define HEAD_FORMAT "GET /api.php?key=free&appid=0&msg=翻译武汉加油 HTTP/1.1\r\nHost:api.qingyunke.com\r\n\r\n"

static const char *TAG = "example";

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n\r\n";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:                               //系统STA已经连接到AP后，获取IP（连接成功事件）
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);    //设置wifi事件组的位
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:                         //断开连接事件
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);     //打印断开连接的原因
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {  //如果是因为速率标准不支持
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);//指定协议类型
        }
        esp_wifi_connect();     //重新连接
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);  //清除wifi事件标志组的连接位
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {           //填入wifi信息
        .sta = {                            //sta模式
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void my_http_task(void *arg)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[1024];

    while(1) {

        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            false, true, portMAX_DELAY);
        ESP_LOGI(TAG, "Connected to AP");

        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));
        
        s = client_socket_IPv4_set(80, "47.107.120.234");
        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);
        
        if (write(s, HEAD_FORMAT, strlen(HEAD_FORMAT)) < 0) {        //发送HTTP请求
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");
        
        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");
        
        memset(recv_buf, 0 ,sizeof(recv_buf));
        r = read(s, recv_buf, 1024);
        printf("recv: %s\r\n", recv_buf);
        
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        
        close(s);
        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
        }

}

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    xTaskCreate(&my_http_task, "my_http_task", 16384, NULL, 5, NULL);
}

