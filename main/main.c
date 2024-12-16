/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "motor.h"
#include "led_strip.h"
#include "esp_console.h"
#include "cmd_i2ctools.h"

#define LED_STRIP_GPIO 5
#define LED_STRIP_NUM 4
#define LED_TASK_STACK_SIZE 2048
#define LED_TASK_PRIO 2
static led_strip_handle_t led_strip;

static void app_leds_init(void);
static void app_i2ctools_init(void);
static void blink_led(void);

typedef struct{
    int method;
    int period;
    led_color_t color;
    TaskHandle_t led_strip_task_handler;
}led_strip_task_conf;

void x_task_led(void *pvparam)
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

void app_main(void)
{
    printf("Hello world!\n");
    app_leds_init();
    
    MOTOR_init();
    app_i2ctools_init();
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void app_leds_init(void)
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
    led_strip_task_conf led_strip_conf;
    led_strip_conf.method = 1;
    led_strip_conf.period = 500;
    led_strip_conf.color = (led_color_t){255,0,255};
    xTaskCreate(x_task_led, "task_led",LED_TASK_STACK_SIZE,&led_strip_conf,LED_TASK_PRIO,
                &(led_strip_conf.led_strip_task_handler));
}

static void app_i2ctools_init(void)
{
    static gpio_num_t i2c_gpio_sda = 33;
    static gpio_num_t i2c_gpio_scl = 34;

    static i2c_port_t i2c_port = I2C_NUM_0;

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "i2c-tools>";
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = i2c_port,
        .scl_io_num = i2c_gpio_scl,
        .sda_io_num = i2c_gpio_sda,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &tool_bus_handle));

    register_i2ctools();

    printf("\n ==============================================================\n");
    printf(" |             Steps to Use i2c-tools                         |\n");
    printf(" |                                                            |\n");
    printf(" |  1. Try 'help', check all supported commands               |\n");
    printf(" |  2. Try 'i2cconfig' to configure your I2C bus              |\n");
    printf(" |  3. Try 'i2cdetect' to scan devices on the bus             |\n");
    printf(" |  4. Try 'i2cget' to get the content of specific register   |\n");
    printf(" |  5. Try 'i2cset' to set the value of specific register     |\n");
    printf(" |  6. Try 'i2cdump' to dump all the register (Experiment)    |\n");
    printf(" |                                                            |\n");
    printf(" ==============================================================\n\n");

    i2c_device_config_t pca_9548a_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x70,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev_handle;
    uint8_t data_wr = 0x01;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(tool_bus_handle, &pca_9548a_dev_cfg, &dev_handle));

    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, &data_wr, 1, -1));
    // start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}