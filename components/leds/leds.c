#include <stdio.h>
#include "leds.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static led_strip_handle_t led_strip;
/**
 * @brief 灯带控制的rtos线程
 * 
 * @param pvparam 线程参数
 */
static void x_task_led(void *pvparam);

/**
 * @brief 更新led灯带
 * 
 * @param led_strip_conf 灯带配置结构体，写入更新的模式，颜色与频率
 */
void leds_update(led_strip_task_conf* led_strip_conf)
{
    vTaskDelete(led_strip_conf->led_strip_task_handler);
    xTaskCreate(x_task_led, "task_led",LED_TASK_STACK_SIZE,led_strip_conf,LED_TASK_PRIO,
                &(led_strip_conf->led_strip_task_handler));
}

/**
 * @brief 初始化led灯带
 * 
 * @param led_strip_conf 灯带配置结构体，写入初始的模式，颜色与频率
 */
void leds_init(led_strip_task_conf* led_strip_conf)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_NUM, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
    xTaskCreate(x_task_led, "task_led",LED_TASK_STACK_SIZE,led_strip_conf,LED_TASK_PRIO,
                &(led_strip_conf->led_strip_task_handler));
}

/**
 * @brief 灯带控制的rtos线程
 * 
 * @param pvparam 线程参数
 */
static void x_task_led(void *pvparam)
{
    led_color_t color = ((led_strip_task_conf*)pvparam)->color;
    int method = ((led_strip_task_conf*)pvparam)->method;
    int period = ((led_strip_task_conf*)pvparam)->period;
    while(1)
    {
        switch (method)
        {
        case 0:
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case 1:
            led_strip_set_on(led_strip, color);
            vTaskDelay(pdMS_TO_TICKS(period));
            break;
        case 2:
            led_strip_blink(led_strip, color);
            vTaskDelay(pdMS_TO_TICKS(period/2));
            break;
        case 3:
            led_strip_set_breath(led_strip, color);
            vTaskDelay(pdMS_TO_TICKS(period/2));
            break;
        default:
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        }        
    }
}