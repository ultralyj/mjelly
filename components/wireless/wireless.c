#include <stdio.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include <errno.h>
#include <netdb.h>
#include "wireless.h"
#include "tcp_client.h"
#include "wifi_phi.h"
#include "nvs_flash.h"
#include "freertos/task.h"

static const char *TAG = "wireless";

static uint8_t is_connected = 0;
int sock;

void wireless_send(const char *_payload)
{
    if (is_connected)
    {
        int err = send(sock, _payload, strlen(_payload), 0);
        if (err < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }
    }
}

void wireless_receive(char *_buffer, size_t _size)
{
    if (is_connected)
    {
        int len = recv(sock, _buffer, _size - 1, MSG_DONTWAIT);
        if (len < 0)
        {
            // ESP_LOGE(TAG, "recv failed: errno %d", errno);
        }
        else
        {
            _buffer[len] = 0;
            ESP_LOGI(TAG, "Received %d bytes from server:", len);
            ESP_LOGI(TAG, "%s", _buffer);
        }
    }
}

void wireless_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = "",
        },
    };
    wifi_init_sta(&wifi_config);

    while(tcp_client(HOST_IP_ADDR, PORT, &sock))
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    is_connected = 1;
}