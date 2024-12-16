/**
 * @file motor.h
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief 电机驱动模块的头文件
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/pulse_cnt.h"
#include "bdc_motor.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MOTOR_NUM 3
#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define BDC_MCPWM_FREQ_HZ             25000    // 25KHz PWM
#define BDC_MCPWM_DUTY_TICK_MAX       (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks

#define MOTOR0_IN1_GPIO              20
#define MOTOR0_IN2_GPIO              21
#define MOTOR1_IN1_GPIO              19
#define MOTOR1_IN2_GPIO              18
#define MOTOR2_IN1_GPIO              13
#define MOTOR2_IN2_GPIO              14

#define MOTOR0_ENCA_GPIO             8
#define MOTOR0_ENCB_GPIO             7
#define MOTOR1_ENCA_GPIO             10
#define MOTOR1_ENCB_GPIO             9
#define MOTOR2_ENCA_GPIO             12
#define MOTOR2_ENCB_GPIO             11

#define BDC_ENCODER_PCNT_HIGH_LIMIT   1000
#define BDC_ENCODER_PCNT_LOW_LIMIT    -1000
#define BDC_ENCODER_PCNT_FILTER       1000
#define MOTOR_LOOP_PERIOD_MS          500   // calculate the motor speed every 10ms
#define BDC_PID_EXPECT_SPEED          400  // expected motor speed, in the pulses counted by the rotary encoder

#define MOTOR0_ADC_GPIO ADC_CHANNEL_0
#define MOTOR1_ADC_GPIO ADC_CHANNEL_1
#define MOTOR2_ADC_GPIO ADC_CHANNEL_2

typedef struct {
    bdc_motor_handle_t motor;
    pcnt_unit_handle_t pcnt_encoder;
    int group_id;
} motor_control_context_t;


/**
 * @brief 电机模块初始化
 * 
 */
void MOTOR_init(void);

#ifdef __cplusplus
}
#endif


