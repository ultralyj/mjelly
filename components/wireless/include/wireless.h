/**
 * @file wireless.h
 * @author ultralyj (yijie_luo@tongji.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2025-02-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __WIRELESS_H__
#define __WIRELESS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    #define HOST_IP_ADDR "192.168.137.1"
    #define EXAMPLE_ESP_WIFI_SSID "BLUE"
    #define EXAMPLE_ESP_WIFI_PASS "qwertyui"
    #define PORT 8080
    
    void wireless_send(const char *_payload);
    void wireless_receive(char *_buffer, size_t _size);
    void wireless_init(void);

#ifdef __cplusplus
}
#endif

#endif
