/**
 * @file tcp_client.c
 * @author ultralyj (yijie_luo@tongji.edu.cn)
 * @brief
 * @version 0.1
 * @date 2025-02-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "tcp_client.h"

#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_event.h"


static const char *TAG = "tcp_client";
static const char *payload = "Message from ESP32 ";
static uint8_t is_connected = 0;

int tcp_client(char *host_ip, uint16_t port, int* sock)
{
    char rx_buffer[128];
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    *sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (*sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }
    int flag = 1;
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, port);
    setsockopt(*sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    int err = connect(*sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return -2;
    }
    ESP_LOGI(TAG, "Successfully connected");
    return 0;
}
