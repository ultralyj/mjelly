/**
 * @file wifi.h
 * @author ultralyj (yijie_luo@tongji.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2025-02-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __WIFI_H__
#define __WIFI_H__

#include <stdint.h>
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void wireless_wifi_sta(wifi_config_t* wifi_config);
#ifdef __cplusplus
}
#endif

#endif
