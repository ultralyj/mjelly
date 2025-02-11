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
#include "bdc_motor.h"
#include "pid.h"
#include "driver/uart.h"
#include <esp_console.h>
#include <mconsole.h>
//#include "uart.h"
// #include "esp_system.h"
// #include "esp_log.h"
// #include "esp_spiffs.h"

static const char TAG[] = "main";
// extern motor_control_context_t motor_ctrl_ctx[3];
//void save_to_csv(void);
//volatile uint8_t received_char = 0;
static void app_console_startup(void);
//static void sensor_measure_read(void);
//static void demo(void);

// 每个通道的每个传感器的漂移值
// long int x_avg[0x4+1][4] = {0};  // 3个通道，每个通道4个传感器
// long int y_avg[0x4+1][4] = {0};
// long int z_avg[0x4+1][4] = {0};
// int sample_count[0x4+1][4] = {0};  // 存储每个传感器的采样次数

mjd_mlx90393_config_t mlx90393_handle[4];
const uint8_t mlx90393_addr_set[4] = {0x0C, 0x0D, 0x0E, 0x0F};
const mjd_mlx90393_metrics_selector_t mlx90393_selector = {true, true, true, true};

bool sensor_present[0x4+1][4] = {0};

void app_main(void)
{
    printf("Mjelly v0.3b3\n");
    /* 电机驱动与检测模组初始化 */
    MOTOR_init();

    /* 配置pca9548a */
    i2cmux_init();

//    cur_pulse_count_0 = 0;

    /* 配置mlx90393 */
    for (size_t channel = I2CMUX_CHANNEL0_ON; channel <= I2CMUX_CHANNEL2_ON; channel <<= 1)
    {
        /* 依次选择三组mlx90393，每组4个 */
        i2cmux_set(channel);
        for (uint8_t i = 0; i < 4; i++)
        {
            /* 检测mlx90393是否存在 */
            esp_err_t ret = i2cmux_probe(I2C_MUX_PORT, mlx90393_addr_set[i], 500);
            if (ret != ESP_OK)
            {
                sensor_present[channel][i] = false;
                ESP_LOGE(TAG, "cannot find i2c device: mlx90393[CH:%d-%d], please check the system!", channel, i);
                continue;
            }
            sensor_present[channel][i] = true;
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
    leds_handle.color = (led_color_t){153, 0, 204};
    leds_handle.method = LEDS_METHOD_ON;
    leds_handle.period = 50;
    leds_init(&leds_handle);

    /* 启动命令行 */
    app_console_startup();
    while (1)
    {   
        //app_console_startup();
        //sensor_measure_read();
        //printf("velocity_2: %d\r\n", velocity_2);
        //printf("velocity_0: %d, velocity_1: %d, velocity_2: %d\r\n", velocity_0, velocity_1, velocity_2);
        //printf("Current Pulse Count_0: %d, Current Pulse Count_1: %d, Current Pulse Count_2: %d\r\n", cur_pulse_count_0, cur_pulse_count_1, cur_pulse_count_2);
        
        vTaskDelay(pdMS_TO_TICKS(10000));
//        demo();

        //cnn(model,in,out)
        // bdc_motor_set_speed(motor_ctrl_ctx[1].motor,400);//290 相当于最开始实验测试的6v
    }

}


static void app_console_startup(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    repl_config.prompt = "mjelly>";

    // install console REPL environment
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    register_mconsole();

    printf("\n ==================================================================\n");
    printf(" |             Steps to Use motor-control-tools                     |\n");
    printf(" |                                                                  |\n");
    printf(" |  1. Try 'help', check all supported commands                     |\n");
    printf(" |  2. Try 'rev -r 0 or 1' to adjust the motor                      |\n");
    printf(" |  3. Try 'ctr -i 0 or 1 or 2 or 3 -s 0-400' to control the motors |\n");
    printf(" |  4. Try 'sen -i 0' to aquire the sensing data                    |\n");
    printf(" ====================================================================\n\n");

    // start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

static void demo(void){
    /*单独正转 */
    bdc_motor_set_speed(motor_ctrl_ctx[0].motor, 400);
    vTaskDelay(pdMS_TO_TICKS(7000));
    bdc_motor_set_speed(motor_ctrl_ctx[0].motor, 0);

    bdc_motor_set_speed(motor_ctrl_ctx[1].motor, 400);
    vTaskDelay(pdMS_TO_TICKS(7000));
    bdc_motor_set_speed(motor_ctrl_ctx[1].motor, 0);

    bdc_motor_set_speed(motor_ctrl_ctx[2].motor, 400);
    vTaskDelay(pdMS_TO_TICKS(7000));
    bdc_motor_set_speed(motor_ctrl_ctx[2].motor, 0);

    /*1，2电机反转*/
    bdc_motor_forward(motor_ctrl_ctx[1].motor);
    bdc_motor_forward(motor_ctrl_ctx[2].motor);
    bdc_motor_set_speed(motor_ctrl_ctx[1].motor, 400);
    bdc_motor_set_speed(motor_ctrl_ctx[2].motor, 400);
    vTaskDelay(pdMS_TO_TICKS(7000));
    bdc_motor_set_speed(motor_ctrl_ctx[1].motor, 0);
    bdc_motor_set_speed(motor_ctrl_ctx[2].motor, 0);

    /*0电机继续正转*/
    bdc_motor_set_speed(motor_ctrl_ctx[0].motor, 400);
    vTaskDelay(pdMS_TO_TICKS(20000));

    /*0电机返回*/
    bdc_motor_reverse(motor_ctrl_ctx[0].motor);
    vTaskDelay(pdMS_TO_TICKS(27000));
    bdc_motor_set_speed(motor_ctrl_ctx[0].motor, 0);

    bdc_motor_forward(motor_ctrl_ctx[0].motor);
    bdc_motor_reverse(motor_ctrl_ctx[1].motor);
    bdc_motor_reverse(motor_ctrl_ctx[2].motor);
}

// static void sensor_measure_read(void)
// {
//             /*ml90393开始测量*/
//     for (uint8_t channel = I2CMUX_CHANNEL0_ON; channel <= I2CMUX_CHANNEL2_ON; channel <<= 1)
//     {
//         i2cmux_set(channel);
//         for (uint8_t i = 0; i < 4; i++)
//         {
//             if (!sensor_present[channel][i])
//             {
//                 continue; // 跳过不存在的传感器
//             }
//             esp_err_t ret = mjd_mlx90393_cmd_start_measurement(&mlx90393_handle[i]);
//             if (ret != ESP_OK)
//             {
//                 ESP_LOGE(TAG, "Failed to start measurement for sensor %d", i);
//                 continue;
//             }
//         }
//     }

//         /*延时*/
//     vTaskDelay(pdMS_TO_TICKS(20));

//         /*ml90393开始读取数据并保存*/
//     mjd_mlx90393_data_t data[4];
//     char output_data[512] = {0}; // 假设最大长度足够存储数据

//     for (uint8_t channel = I2CMUX_CHANNEL0_ON; channel <= I2CMUX_CHANNEL2_ON; channel <<= 1)
//     {
//         i2cmux_set(channel);

//         // // 用于构建输出的字符串，存储每个传感器的数据
//         //     char output_data[512] = {0}; // 假设最大长度足够存储数据

//         for (uint8_t i = 0; i < 4; i++)
//         {
//             if (!sensor_present[channel][i])
//             {
//                 continue; // 跳过不存在的传感器
//             }
//             esp_err_t ret = mjd_mlx90393_cmd_read_measurement(&mlx90393_handle[i], &data[i]);
//             if (ret == ESP_OK)
//             {   
//                 if (sample_count[channel][i] < 10)  // 前10次采集，计算每个传感器的平均值
//                 {
//                     x_avg[channel][i] += data[i].x;
//                     y_avg[channel][i] += data[i].y;
//                     z_avg[channel][i] += data[i].z;
//                     sample_count[channel][i]++;
//                 }
//                 else  // 从第11次起，打印并减去平均值
//                 {
//                        // 从第 11 次采集起，打印数据并减去漂移平均值
//                     snprintf(output_data + strlen(output_data), sizeof(output_data) - strlen(output_data),
//                              "%ld, %ld, %ld, ", 
//                             (long int)(data[i].x - x_avg[channel][i] / 10),
//                             (long int)(data[i].y - y_avg[channel][i] / 10),
//                             (long int)(data[i].z - z_avg[channel][i] / 10));
//                             // ESP_LOGI(TAG, "[CH:%d-%d] Temperature: %ld, X: %ld, Y: %ld, Z: %ld", 
//                             //       channel, i, 
//                             //       (long int)data[i].t, 
//                             //       (long int)data[i].x - x_avg[channel][i] / 5, 
//                             //       (long int)data[i].y - y_avg[channel][i] / 5, 
//                             //       (long int)data[i].z - z_avg[channel][i] / 5);
//                 }
//                     //ESP_LOGI(TAG, "[CH:%d-%d]Temperature: %d, X: %d, Y: %d, Z: %d", channel, i, data[i].t, data[i].x, data[i].y, data[i].z); // 打印传感器数据
//             }
//             else
//             {
//                 ESP_LOGE(TAG, "Failed to read data from MLX90393[CH:%d-%d]", channel, i);
//             }
//         }
//     }
//     printf("%s\n", output_data);
//         //printf("velocity_0: %d, velocity_1: %d, velocity_2: %d\r\n", velocity_0, velocity_1, velocity_2);
//     vTaskDelay(pdMS_TO_TICKS(10));
// }

