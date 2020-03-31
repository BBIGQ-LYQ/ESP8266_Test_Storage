deps_config := \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/app_update/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/aws_iot/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/console/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/esp8266/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/esp_http_client/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/esp_http_server/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/freertos/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/libsodium/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/log/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/lwip/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/mdns/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/mqtt/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/newlib/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/pthread/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/spiffs/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/ssl/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/tcpip_adapter/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/util/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/vfs/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/wifi_provisioning/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/wpa_supplicant/Kconfig \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/bootloader/Kconfig.projbuild \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/esptool_py/Kconfig.projbuild \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/components/partition_table/Kconfig.projbuild \
	/home/bbigq/ESP-Work/ESP8266_RTOS_SDK/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
