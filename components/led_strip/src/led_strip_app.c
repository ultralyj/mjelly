/**
 * @file led_strip_app.c
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief
 * @version 0.1
 * @date 2024-12-11
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "led_strip.h"
#include "led_strip_interface.h"
#include "led_strip_rmt_encoder.h"

typedef struct
{
    led_strip_t base;
    rmt_channel_handle_t rmt_chan;
    rmt_encoder_handle_t strip_encoder;
    uint32_t strip_len;
    uint8_t bytes_per_pixel;
    uint8_t pixel_buf[];
} led_strip_rmt_obj;

void led_strip_set_on(led_strip_handle_t strip, led_color_t color)
{
    led_strip_rmt_obj *rmt_strip = __containerof(strip, led_strip_rmt_obj, base);
    for (int i = 0; i < rmt_strip->strip_len; i++)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, i, color.red, color.green, color.blue));
    }
    ESP_ERROR_CHECK(strip->refresh(strip));
}

void led_strip_set_breath(led_strip_handle_t strip, led_color_t color)
{
    led_strip_rmt_obj *rmt_strip = __containerof(strip, led_strip_rmt_obj, base);
    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j < rmt_strip->strip_len; j++)
        {
            ESP_ERROR_CHECK(strip->set_pixel(strip, j, (color.red * i) / 255, (color.green * i) / 255, (color.blue * i) / 255));
        }
        ESP_ERROR_CHECK(strip->refresh(strip));
        vTaskDelay(pdMS_TO_TICKS(10)); // 调整呼吸速度
    }
    for (int i = 255; i >= 0; i--)
    {
        for (int j = 0; j < rmt_strip->strip_len; j++)
        {
            ESP_ERROR_CHECK(strip->set_pixel(strip, j, (color.red * i) / 255, (color.green * i) / 255, (color.blue * i) / 255));
        }
        ESP_ERROR_CHECK(strip->refresh(strip));
        vTaskDelay(pdMS_TO_TICKS(10)); // 调整呼吸速度
    }
}


void led_strip_blink(led_strip_handle_t strip, led_color_t color)
{
    static uint8_t s_led_state = 0;
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_on(strip, color);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(strip);
    }
    s_led_state = !s_led_state;
}