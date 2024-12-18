/**
 * @file i2c_mux.c
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2024-12-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include "i2c_mux.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

i2c_master_bus_handle_t i2c_bus_handle_g;
i2c_master_dev_handle_t pca9548a_handle;

static const char TAG[] = "i2c_mux";

/**
 * @brief 初始化I2C总线，并检测、配置设备PCA9548A
 * 
 */
void i2cmux_init(void)
{
    /* 初始化I2C总线 */
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MUX_PORT,
        .scl_io_num = I2C_GPIO_SCL,
        .sda_io_num = I2C_GPIO_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle_g));

    /* 检测PCA9548A是否存在 */
    esp_err_t ret = i2c_master_probe(i2c_bus_handle_g, PCA9548A_ADDR, I2C_TIMEOUT_VALUE_MS);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "cannot find i2c device: pca9548a, please check the system!");
        return;
    }
    /* 初始化I2C设备：PCA9548A */
    i2c_device_config_t pca_9548a_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCA9548A_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle_g, &pca_9548a_dev_cfg, &pca9548a_handle));

    i2cmux_set(I2CMUX_CHANNEL_ALLOFF);
}

/**
 * @brief 配置I2C选通通道
 * 
 * @param sel 通道选择
 */
void i2cmux_set(i2cmux_channel_sel sel)
{
    uint8_t channel_sel = (uint8_t)sel;
    ESP_ERROR_CHECK(i2c_master_transmit(pca9548a_handle, &channel_sel, 1, -1));
}
