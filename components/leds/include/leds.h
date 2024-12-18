/**
 * @file leds.h
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief 灯带模块的头文件
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "led_strip.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LED_STRIP_GPIO 5
#define LED_STRIP_NUM 4
#define LED_TASK_STACK_SIZE 2048
#define LED_TASK_PRIO 2

/**
 * @brief led灯带显示模式配置
 * 
 */
typedef struct{
    int method;
    int period;
    led_color_t color;
    TaskHandle_t led_strip_task_handler;
}led_strip_task_conf;

typedef enum{
    LEDS_METHOD_OFF = 0,
    LEDS_METHOD_ON = 1,
    LEDS_METHOD_BLINK = 2,
    LEDS_METHOD_BREATH = 3
}led_strip_method_def;

/**
 * @brief 初始化led灯带
 * 
 * @param led_strip_conf 灯带配置结构体，写入初始的模式，颜色与频率
 */
void leds_init(led_strip_task_conf* led_strip_conf);

/**
 * @brief 更新led灯带
 * 
 * @param led_strip_conf 灯带配置结构体，写入更新的模式，颜色与频率
 */
void leds_update(led_strip_task_conf* led_strip_conf);
#ifdef __cplusplus
}
#endif


