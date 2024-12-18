/**
 * @file i2cmux.h
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief pca9548a模块的头文件
 * @version 0.1
 * @date 2024-12-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_GPIO_SDA 33
#define I2C_GPIO_SCL 34
#define I2C_MUX_PORT I2C_NUM_0
#define I2C_TIMEOUT_VALUE_MS (50)

#define PCA9548A_ADDR 0x70    

typedef enum
{
    I2CMUX_CHANNEL_ALLOFF = 0x00,
    I2CMUX_CHANNEL0_ON = 0x01,
    I2CMUX_CHANNEL1_ON = 0x02,
    I2CMUX_CHANNEL2_ON = 0x04,
    I2CMUX_CHANNEL_ALLON = 0xFF
}i2cmux_channel_sel;

extern i2c_master_bus_handle_t i2c_bus_handle_g;

/**
 * @brief 初始化I2C总线，并检测、配置设备PCA9548A
 * 
 */
void i2cmux_init(void);

/**
 * @brief 配置I2C选通通道
 * 
 * @param sel 通道选择
 */
void i2cmux_set(i2cmux_channel_sel sel);

#ifdef __cplusplus
}
#endif


