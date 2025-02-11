/**
 * @file motor.c
 * @author Luo Yijie (yijie_luo@tongji.edu.cn)
 * @brief 电机驱动，包含电机互补pwm输出控制，编码器位置读取，电流传感器读取
 * @version 0.1
 * @date 2024-12-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "motor.h"
#include "bdc_motor.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "pid.h"

static const char *TAG = "MOTOR";

motor_control_context_t motor_ctrl_ctx[3];
adc_oneshot_unit_handle_t adc1_handle;

// 定义全局变量，volatile 防止编译器优化
volatile int cur_pulse_count_0 = 0;
volatile int cur_pulse_count_1 = 0;
volatile int cur_pulse_count_2 = 0;

// 用于保存上次脉冲数，计算脉冲数差（速度）
volatile int last_pulse_count_0 = 0;
volatile int last_pulse_count_1 = 0;
volatile int last_pulse_count_2 = 0;

// 速度值（脉冲数差）
volatile int velocity_0 = 0;
volatile int velocity_1 = 0;
volatile int velocity_2 = 0;

/**
 * @brief 配置单个电机的互补pwm驱动(由mcpwm外设输出)
 * 
 * @param motor 电机控制句柄
 * @param pwma 电机驱动控制引脚1
 * @param pwmb 电机驱动控制引脚2
 */
static void MOTOR_Bdc_Unit_Init(motor_control_context_t *motor, const int pwma, const int pwmb);

/**
 * @brief 配置单个编码器驱动
 * 
 * @param motor 电机控制句柄
 * @param enca 正交编码器A相引脚序号
 * @param encb 正交编码器B相引脚序号
 */
static void MOTOR_Enc_Unit_Init(motor_control_context_t *motor, const int enca, const int encb);

/**
 * @brief 初始化电机电流检测ADC
 * 
 */
static void MOTOR_Adc_Unit_Init(void);
/**
 * @brief 电机控制定时器中断
 * 
 * @param args 
 */
static void Motor_Control_Callback(void *args);

/**
 * @brief 电机模块初始化
 * 
 */
void MOTOR_init(void)
{
    const int MOTOR_IN1_GPIO_SET[MOTOR_NUM] = {MOTOR0_IN1_GPIO, MOTOR1_IN1_GPIO, MOTOR2_IN1_GPIO};
    const int MOTOR_IN2_GPIO_SET[MOTOR_NUM] = {MOTOR0_IN2_GPIO, MOTOR1_IN2_GPIO, MOTOR2_IN2_GPIO};
    const int MOTOR_ENCA_GPIO_SET[MOTOR_NUM] = {MOTOR0_ENCA_GPIO, MOTOR1_ENCA_GPIO, MOTOR2_ENCA_GPIO};
    const int MOTOR_ENCB_GPIO_SET[MOTOR_NUM] = {MOTOR0_ENCB_GPIO, MOTOR1_ENCB_GPIO, MOTOR2_ENCB_GPIO};
    
    /* 配置三个电机的互补pwm与正交编码器驱动 */
    for (int i = 0; i < MOTOR_NUM; i++)
    {
        motor_ctrl_ctx[i].group_id = 0;
        MOTOR_Bdc_Unit_Init(&motor_ctrl_ctx[i],MOTOR_IN1_GPIO_SET[i],MOTOR_IN2_GPIO_SET[i]);
        MOTOR_Enc_Unit_Init(&motor_ctrl_ctx[i],MOTOR_ENCA_GPIO_SET[i],MOTOR_ENCB_GPIO_SET[i]);
    }
    /* 初始化电机电流检测 */
    MOTOR_Adc_Unit_Init();
    /* 设置电机控制定时器 */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = Motor_Control_Callback,
        .name = "motor_loop"
    };
    esp_timer_handle_t motor_loop_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &motor_loop_timer));
    /* 启动定时器 */
    ESP_LOGD(TAG, "Start motor speed loop");
    ESP_ERROR_CHECK(esp_timer_start_periodic(motor_loop_timer, MOTOR_LOOP_PERIOD_MS * 1000));

    bdc_motor_forward(motor_ctrl_ctx[0].motor);
    bdc_motor_reverse(motor_ctrl_ctx[1].motor);
    bdc_motor_reverse(motor_ctrl_ctx[2].motor);

    // bdc_motor_reverse(motor_ctrl_ctx[0].motor);
    // bdc_motor_reverse(motor_ctrl_ctx[1].motor);
    // bdc_motor_reverse(motor_ctrl_ctx[2].motor);
    
    bdc_motor_set_speed(motor_ctrl_ctx[0].motor,0);
    bdc_motor_set_speed(motor_ctrl_ctx[1].motor,0);
    bdc_motor_set_speed(motor_ctrl_ctx[2].motor,0);
}

/**
 * @brief 配置单个电机的互补pwm驱动(由mcpwm外设输出)
 * 
 * @param motor 电机控制句柄
 * @param pwma 电机驱动控制引脚1
 * @param pwmb 电机驱动控制引脚2
 */
static void MOTOR_Bdc_Unit_Init(motor_control_context_t *motor, const int pwma, const int pwmb)
{
    bdc_motor_handle_t motor_handle = NULL;
    /* 配置电机驱动 */
    bdc_motor_config_t motor_config = {
        .pwm_freq_hz = BDC_MCPWM_FREQ_HZ,
        .pwma_gpio_num = pwma,
        .pwmb_gpio_num = pwmb,
    };
    bdc_motor_mcpwm_config_t mcpwm_config = {
        .group_id = motor->group_id,
        .resolution_hz = BDC_MCPWM_TIMER_RESOLUTION_HZ,
    };
    
    ESP_ERROR_CHECK(bdc_motor_new_mcpwm_device(&motor_config, &mcpwm_config, &motor_handle));

    ESP_LOGD(TAG, "Enable motor");
    ESP_ERROR_CHECK(bdc_motor_enable(motor_handle));
    ESP_LOGD(TAG, "Forward motor");
    ESP_ERROR_CHECK(bdc_motor_forward(motor_handle));
    motor->motor = motor_handle;
}

/**
 * @brief 配置单个编码器驱动
 * 
 * @param motor 电机控制句柄
 * @param enca 正交编码器A相引脚序号
 * @param encb 正交编码器B相引脚序号
 */
static void MOTOR_Enc_Unit_Init(motor_control_context_t *motor, const int enca, const int encb)
{
    /* 初始化pcnt */
    ESP_LOGD(TAG, "Init pcnt driver to decode rotary signal");
    pcnt_unit_config_t unit_config = {
        .high_limit = BDC_ENCODER_PCNT_HIGH_LIMIT,
        .low_limit = BDC_ENCODER_PCNT_LOW_LIMIT,
        .flags.accum_count = true, // enable counter accumulation
    };
    pcnt_unit_handle_t pcnt_unit = NULL;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
    /* 设置pcnt毛刺滤波器 */
    ESP_LOGD(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = BDC_ENCODER_PCNT_FILTER,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    /* 配置pcnt通道，一共两个通道，a检测上升沿，b检测下降沿（注意：两者引脚相反） */
    ESP_LOGD(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = enca,
        .level_gpio_num = encb,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = encb,
        .level_gpio_num = enca,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    /* 添加观察点 */
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, BDC_ENCODER_PCNT_HIGH_LIMIT));  // 设定高限制观察点
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, BDC_ENCODER_PCNT_LOW_LIMIT));   // 设定低限制观察点

    /* 设置pcnt中断回调 */
//    ESP_ERROR_CHECK(pcnt_unit_register_isr_handler(pcnt_unit, pcnt_isr_handler, (void *)pcnt_unit));


    /* 启动pcnt模块 */
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_LOGD(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
    /* 更新控制句柄 */
    motor->pcnt_encoder = pcnt_unit;
}

/**
 * @brief 初始化电机电流检测ADC
 * 
 */
static void MOTOR_Adc_Unit_Init(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, // 12bit位宽
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MOTOR0_ADC_GPIO, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MOTOR1_ADC_GPIO, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MOTOR2_ADC_GPIO, &config));
}
/**
 * @brief 电机控制定时器中断
 * 
 * @param args 
 */
static void Motor_Control_Callback(void *args)
{
//    int cur_pulse_count_0, cur_pulse_count_1, cur_pulse_count_2 = 0;

// pid_control();//PID应该放到这里
    static int count0, count1, count2 = 0;

    pcnt_unit_handle_t pcnt_unit_0 = motor_ctrl_ctx[0].pcnt_encoder;
    pcnt_unit_handle_t pcnt_unit_1 = motor_ctrl_ctx[1].pcnt_encoder;
    pcnt_unit_handle_t pcnt_unit_2 = motor_ctrl_ctx[2].pcnt_encoder;

    // 获取当前脉冲数
    pcnt_unit_get_count(pcnt_unit_0, &cur_pulse_count_0);
    pcnt_unit_get_count(pcnt_unit_1, &cur_pulse_count_1);
    pcnt_unit_get_count(pcnt_unit_2, &cur_pulse_count_2);

    velocity_0 = cur_pulse_count_0 - last_pulse_count_0;
    velocity_1 = cur_pulse_count_1 - last_pulse_count_1;
    velocity_2 = cur_pulse_count_2 - last_pulse_count_2;

    last_pulse_count_0 = cur_pulse_count_0;
    last_pulse_count_1 = cur_pulse_count_1;
    last_pulse_count_2 = cur_pulse_count_2;

    count0 ++;
    count1 ++;
    count2 ++;

    //pid_control();
    //pcnt_unit_clear_count(pcnt_unit);
   // 打印当前脉冲数、溢出次数和总脉冲数
//   printf("Current Pulse Count_0: %d, Current Pulse Count_1: %d, Current Pulse Count_2: %d\r\n", cur_pulse_count_0, cur_pulse_count_1, cur_pulse_count_2);


    if (count0 >= 10){
        if (cur_pulse_count_0 >= -1000 && cur_pulse_count_0 <= 8000) {
            bdc_motor_set_speed(motor_ctrl_ctx[0].motor,0);  // 假设这是停止电机的函数
            pcnt_unit_clear_count(pcnt_unit_0);
            count0 = 0;
        }
    }

    if (count1 >= 10){
        if (cur_pulse_count_1 >= -1000 && cur_pulse_count_1 <= 8000) {
            bdc_motor_set_speed(motor_ctrl_ctx[1].motor,0);  // 假设这是停止电机的函数
            pcnt_unit_clear_count(pcnt_unit_1);
            count1 = 0;
        }
    }

    if (count2 >=10){
        if (cur_pulse_count_2 >= -1000 && cur_pulse_count_2 <= 8000) {
            bdc_motor_set_speed(motor_ctrl_ctx[2].motor,0);  // 假设这是停止电机的函数
            pcnt_unit_clear_count(pcnt_unit_2);
            count2 = 0;
        }
    }



 
    // int adcv;
    // ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, MOTOR0_ADC_GPIO, &adcv));
    // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, MOTOR0_ADC_GPIO, adcv);
}

