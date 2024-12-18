/**
 * @file main.c
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2024-12-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "motor.h"
#include "esp_console.h"
#include "cmd_i2ctools.h"
#include "leds.h"
#include "i2c_mux.h"
#include "mlx90393.h"

static const char TAG[] = "main";

mjd_mlx90393_config_t mlx90393_handle[4];
const mlx90393_addr_set[4] = {0x0C, 0x0D, 0x0E, 0x0F};
const mjd_mlx90393_metrics_selector_t mlx90393_selector = {true, true, true, true};

void app_main(void)
{
    printf("Mjelly v0.3a\n");
    /* 电机驱动与检测模组初始化 */
    MOTOR_init();

    /* 配置pca9548a */
    i2cmux_init();

    /* 配置mlx90393 */
    esp_err_t ret;
    /* 初始化霍尔传感器 */
    
    for (size_t channel = 0; channel <= I2CMUX_CHANNEL2_ON; channel<<=1)
    {
        /* 依次选择三组mlx90393，每组4个 */
        i2cmux_set(channel);
        for (uint8_t i = 0; i < 4; i++)
        {
            /* 检测mlx90393是否存在 */
            esp_err_t ret = i2c_master_probe(i2c_bus_handle_g, mlx90393_addr_set[i], 500);
            if(ret != ESP_OK)
            {
                ESP_LOGE(TAG, "cannot find i2c device: mlx90393[CH:%d-%d], please check the system!",channel,i);
                continue;
            }
            /* 配置，初始化单个mlx90393 */
            mlx90393_handle[i].manage_i2c_driver = false;
            mlx90393_handle[i].i2c_port_num = I2C_NUM_0;
            mlx90393_handle[i].i2c_slave_addr = mlx90393_addr_set[i];
            mlx90393_handle[i].int_gpio_num = GPIO_NUM_NC;
            mlx90393_handle[i].mlx_metrics_selector = mlx90393_selector;
            mlx90393_init(&mlx90393_handle[i]);
        }
    }
    
    /* 配置led灯带 */
    led_strip_task_conf leds_handle;
    leds_handle.color = (led_color_t){216, 56, 21};
    leds_handle.method = LEDS_METHOD_BREATH;
    leds_handle.period = 500;
    leds_init(&leds_handle);
    
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}