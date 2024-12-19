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
    i2c_config_t i2c_conf =
            { 0 };
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.scl_io_num = I2C_GPIO_SCL;
    i2c_conf.sda_io_num = I2C_GPIO_SDA;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_DISABLE; //
    i2c_conf.scl_pullup_en = GPIO_PULLUP_DISABLE; //
    i2c_conf.master.clk_speed = 1000000;

    i2c_param_config(I2C_MUX_PORT, &i2c_conf);

    i2c_driver_install(I2C_MUX_PORT, I2C_MODE_MASTER, 0, 0, 0);
    
    /* 检测PCA9548A是否存在 */
    esp_err_t ret = i2cmux_probe(I2C_MUX_PORT, PCA9548A_ADDR, I2C_TIMEOUT_VALUE_MS);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "cannot find i2c device: pca9548a, please check the system!");
        return;
    }
    /* 初始化I2C设备：PCA9548A */
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
    i2c_master_write_to_device(I2C_MUX_PORT, PCA9548A_ADDR, &sel, 1, pdMS_TO_TICKS(I2C_TIMEOUT_VALUE_MS));
}

/**
 * @brief 检测I2C设备是否存在
 * 
 * @param i2c_num 
 * @param address 
 * @param xfer_timeout_ms 
 */
esp_err_t i2cmux_probe(i2c_port_t i2c_num, uint16_t address, int xfer_timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(xfer_timeout_ms));
    i2c_cmd_link_delete(cmd);
    return ret;
}