#include <stdio.h>
#include "mconsole.h"
#include <esp_console.h>
#include "argtable3/argtable3.h"
#include "bdc_motor.h"
#include "motor.h"
#include "mlx90393.h"
#include "i2c_mux.h"
#include "led_strip.h"

static int do_Reverse_all_motor(int argc, char **argv);
static void Reverse_all_motor(void);
static int do_control_motor(int argc, char **argv);
static void control_motor(void);
static void Sensing_all(void);
static int do_Sensing_all(int argc, char **argv);

static void sensor_measure_read(void);

long int x_avg[0x4+1][4] = {0};  // 3个通道，每个通道4个传感器
long int y_avg[0x4+1][4] = {0};
long int z_avg[0x4+1][4] = {0};
int sample_count[0x4+1][4] = {0};  // 存储每个传感器的采样次数

// const uint8_t mlx90393_addr_set[4] = {0x0C, 0x0D, 0x0E, 0x0F};
// const mjd_mlx90393_metrics_selector_t mlx90393_selector = {true, true, true, true};

// mjd_mlx90393_config_t mlx90393_handle[4];
// bool sensor_present[0x4+1][4] = {0};

static const char TAG[] = "sen";
/**
 * @brief 注册所有命令行命令
 * 
 */
void register_mconsole(void)
{
    Reverse_all_motor();
    control_motor();
    Sensing_all();
}

static struct {
    struct arg_int *sense;
    struct arg_end *end;
} Sensor_args;

static struct {
    struct arg_int *times;
    struct arg_end *end;
} Reverse_args;


static struct {
    struct arg_int *index;  // 电机索引
    struct arg_int *speed;  // 电机转速
    // struct arg_int *direction; // 电机方向 (0 正转, 1 反转)
    struct arg_end *end;
} motor_args;


static void Reverse_all_motor(void)
{
    Reverse_args.times = arg_int0("r", "reverse", "<reverse>", "reverse the motor");
    Reverse_args.end = arg_end(1);
    const esp_console_cmd_t hello_cmd = {
        .command = "rev",
        .help = "reverse the chosed motor",
        .hint = NULL,
        .func = &do_Reverse_all_motor,
        .argtable = &Reverse_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&hello_cmd));
}

static int do_Reverse_all_motor(int argc, char **argv)
{
    /* 解析命令行参数 */
    int nerrors = arg_parse(argc, argv, (void **)&Reverse_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, Reverse_args.end, argv[0]);
        return 0;
    }
    /* 如果参数非空则使用参数提供的次数 */
    int times = Reverse_args.times->count ? Reverse_args.times->ival[0] : 0;

    if(times == 0)
    {
        bdc_motor_forward(motor_ctrl_ctx[0].motor);
        bdc_motor_reverse(motor_ctrl_ctx[1].motor);
        bdc_motor_reverse(motor_ctrl_ctx[2].motor);
        printf("Motor has been set to forward.\n");
    }else{
        bdc_motor_reverse(motor_ctrl_ctx[0].motor);
        bdc_motor_forward(motor_ctrl_ctx[1].motor);
        bdc_motor_forward(motor_ctrl_ctx[2].motor);
        printf("Motor has been set to reverse.\n");
    }

    for (int i = 0; i < 3; i++)
    {
        printf("velocity_0: %d, velocity_1: %d, velocity_2: %d\r\n", velocity_0, velocity_1, velocity_2);
        printf("Current Pulse Count_0: %d, Current Pulse Count_1: %d, Current Pulse Count_2: %d\r\n", cur_pulse_count_0, cur_pulse_count_1, cur_pulse_count_2);
    }
    /* 执行命令 */
    return 0;
}

static void control_motor(void)
{
    motor_args.index = arg_int0("i", "index", "<index>", "Specify motor index (0, 1, or 2)");
    motor_args.speed = arg_int0("s", "speed", "<speed>", "Specify motor speed (0 to stop, 400 for full speed)");
    // motor_args.direction = arg_int0("d", "direction", "<direction>", "Specify motor direction (0 for forward, 1 for reverse)");
    motor_args.end = arg_end(2);

    const esp_console_cmd_t hello_cmd = {
        .command = "ctr",
        .help = "Control motor speed and direction",
        .hint = NULL,
        .func = &do_control_motor,
        .argtable = &motor_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&hello_cmd));
}

static int do_control_motor(int argc, char **argv)
{
    /* 解析命令行参数 */
    int nerrors = arg_parse(argc, argv, (void **)&motor_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, motor_args.end, argv[0]);
        return 0;
    }

    /* 默认将 index 设置为 3，表示停止所有电机 */
    int index = motor_args.index->count ? motor_args.index->ival[0] : 3;

        // 参数校验
    if (index < 0 || index > 3) {
        fprintf(stderr, "Invalid motor index: %d. Must be 0, 1, or 2, or 3.\n", index);
        return 1;
    }

    // 获取速度参数，如果没有提供速度，则默认设置为 0（即停止电机）
    int speed = motor_args.speed->count ? motor_args.speed->ival[0] : 0;

    // // 获取方向参数，默认设置为 0（正转）
    // int direction = motor_args.direction->count ? motor_args.direction->ival[0] : 0;

    // if (direction == 1) {
    //     // 设置为反转，这里假设反转是通过设置电机控制函数的某个参数来实现
    //     bdc_motor_reverse(motor_ctrl_ctx[index].motor);  // 需要实现反转函数
    //     printf("Motor %d has been set to reverse.\n", index);
    // }else {
    //     // 正转（如果方向未提供，则默认正转）
    //     bdc_motor_forward(motor_ctrl_ctx[index].motor);  // 需要实现反转函数
    //     printf("Motor %d has been set to speed %d.\n", index, speed);
    // } 

    // 索引值为 3 时，控制所有电机
    //static bool has_executed = false;
    if (index == 3) {
        //has_executed = true;
        for (int i = 0; i < 3; i++) {
            bdc_motor_set_speed(motor_ctrl_ctx[i].motor, speed);
        }
        for (uint32_t c = 0; c < 500; c++ ){
            sensor_measure_read();
        }
        return 0;
    }

        //
    bdc_motor_set_speed(motor_ctrl_ctx[index].motor, speed);
    printf("Motor %d has been set to speed %d.\n", index, speed);

    for (int i = 0; i < 3; i++)
    {
        printf("velocity_0: %d, velocity_1: %d, velocity_2: %d\r\n", velocity_0, velocity_1, velocity_2);
        printf("Current Pulse Count_0: %d, Current Pulse Count_1: %d, Current Pulse Count_2: %d\r\n", cur_pulse_count_0, cur_pulse_count_1, cur_pulse_count_2);
    }
    /* 执行命令 */
    return 0;
}


static void Sensing_all(void)
{
    Sensor_args.sense = arg_int0("e", "sense", "<sense>", "start sensing");
    Sensor_args.end = arg_end(1);
    const esp_console_cmd_t hello_cmd = {
        .command = "sen",
        .help = "start sensing",
        .hint = NULL,
        .func = &do_Sensing_all,
        .argtable = &Sensor_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&hello_cmd));
}

static int do_Sensing_all(int argc, char **argv)
{
    /* 解析命令行参数 */
    int nerrors = arg_parse(argc, argv, (void **)&Sensor_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, Sensor_args.end, argv[0]);
        return 0;
    }
    /* 如果参数非空则使用参数提供的次数 */
    int sense = Sensor_args.sense->count ? Sensor_args.sense->ival[0] : 0;

    if(sense == 0)
    {   
        for (uint8_t i = 0; i < 200; i++ ){
            sensor_measure_read();
        }
    }

    /* 执行命令 */
    return 0;
}

static void sensor_measure_read(void)
{
            /*ml90393开始测量*/
    for (uint8_t channel = I2CMUX_CHANNEL0_ON; channel <= I2CMUX_CHANNEL2_ON; channel <<= 1)
    {
        i2cmux_set(channel);
        for (uint8_t i = 0; i < 4; i++)
        {
            if (!sensor_present[channel][i])
            {
                continue; // 跳过不存在的传感器
            }
            esp_err_t ret = mjd_mlx90393_cmd_start_measurement(&mlx90393_handle[i]);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to start measurement for sensor %d", i);
                continue;
            }
        }
    }

        /*延时*/
    vTaskDelay(pdMS_TO_TICKS(20));

        /*ml90393开始读取数据并保存*/
    mjd_mlx90393_data_t data[4];
    char output_data[512] = {0}; // 假设最大长度足够存储数据
    const uint32_t threshold =500;
    uint8_t contact_flag = 0;

    // for (uint8_t channel = I2CMUX_CHANNEL0_ON; channel <= I2CMUX_CHANNEL2_ON; channel <<= 1)
    // {
    //     i2cmux_set(channel);

    //     // // 用于构建输出的字符串，存储每个传感器的数据
    //     //     char output_data[512] = {0}; // 假设最大长度足够存储数据

    //     for (uint8_t i = 0; i < 4; i++)
    //     {
    //         if (!sensor_present[channel][i])
    //         {
    //             continue; // 跳过不存在的传感器
    //         }
    //         esp_err_t ret = mjd_mlx90393_cmd_read_measurement(&mlx90393_handle[i], &data[i]);
    //         if (ret == ESP_OK)
    //         {   
    //             if (sample_count[channel][i] < 10)  // 前10次采集，计算每个传感器的平均值
    //             {
    //                 x_avg[channel][i] += data[i].x;
    //                 y_avg[channel][i] += data[i].y;
    //                 z_avg[channel][i] += data[i].z;
    //                 sample_count[channel][i]++;
    //             }
    //             else  // 从第11次起，打印并减去平均值
    //             {
    //                    // 从第 11 次采集起，打印数据并减去漂移平均值
    //                 snprintf(output_data + strlen(output_data), sizeof(output_data) - strlen(output_data),
    //                          "%ld, %ld, %ld, ", 
    //                         (long int)(data[i].x - x_avg[channel][i] / 10),
    //                         (long int)(data[i].y - y_avg[channel][i] / 10),
    //                         (long int)(data[i].z - z_avg[channel][i] / 10));
    //                         // ESP_LOGI(TAG, "[CH:%d-%d] Temperature: %ld, X: %ld, Y: %ld, Z: %ld", 
    //                         //       channel, i, 
    //                         //       (long int)data[i].t, 
    //                         //       (long int)data[i].x - x_avg[channel][i] / 5, 
    //                         //       (long int)data[i].y - y_avg[channel][i] / 5, 
    //                         //       (long int)data[i].z - z_avg[channel][i] / 5);
    //                     if(abs(data[i].z -z_avg[channel][i] / 10) > threshold)
    //                     {
    //                         contact_flag++;
    //                         bdc_motor_set_speed(motor_ctrl_ctx[0].motor, 0);
    //                         bdc_motor_set_speed(motor_ctrl_ctx[1].motor, 0);
    //                         bdc_motor_set_speed(motor_ctrl_ctx[2].motor, 0);
                            
    //                     }
                            
    //             }
    //                 //ESP_LOGI(TAG, "[CH:%d-%d]Temperature: %d, X: %d, Y: %d, Z: %d", channel, i, data[i].t, data[i].x, data[i].y, data[i].z); // 打印传感器数据
    //         }
    //         else
    //         {
    //             ESP_LOGE(TAG, "Failed to read data from MLX90393[CH:%d-%d]", channel, i);
    //         }
    //     }
        
    // }

    for (uint8_t channel = I2CMUX_CHANNEL0_ON; channel <= I2CMUX_CHANNEL2_ON; channel <<= 1)
    {
        i2cmux_set(channel);
        bool stop_motor = false; // 标记是否需要停止对应的电机

        for (uint8_t i = 0; i < 4; i++)
        {
            if (!sensor_present[channel][i])
            {
                continue; // 跳过不存在的传感器
            }

            esp_err_t ret = mjd_mlx90393_cmd_read_measurement(&mlx90393_handle[i], &data[i]);
            if (ret == ESP_OK)
            {   
                if (sample_count[channel][i] < 10)  // 前 10 次采集，计算平均值
                {
                    x_avg[channel][i] += data[i].x;
                    y_avg[channel][i] += data[i].y;
                    z_avg[channel][i] += data[i].z;
                    sample_count[channel][i]++;
                }
                else  // 从第 11 次起，计算去漂移后的数据
                {
                    int32_t z_filtered = (int32_t)(data[i].z - z_avg[channel][i] / 10);

                    snprintf(output_data + strlen(output_data), sizeof(output_data) - strlen(output_data),
                            "%ld, %ld, %ld, ", 
                            (long int)(data[i].x - x_avg[channel][i] / 10),
                            (long int)(data[i].y - y_avg[channel][i] / 10),
                            (long int)z_filtered);

                // 如果当前 channel 上任意传感器的 Z 轴超过阈值，则停止该通道对应的电机
                    if (abs(z_filtered) > threshold)
                    {
                        stop_motor = true;
                    }
                }
            }
            else
            {
                ESP_LOGE(TAG, "Failed to read data from MLX90393[CH:%d-%d]", channel, i);
            }
        }

    // 计算电机索引: channel(1,2,4) -> motor_index(0,1,2)
        int motor_index = (channel == 1) ? 0 : (channel == 2) ? 1 : 2;

    // 如果 stop_motor 为 true，停止对应的 motor
        if (stop_motor)
        {
            contact_flag++;
            bdc_motor_set_speed(motor_ctrl_ctx[motor_index].motor, 0);
        }
    }


    if(contact_flag)
    {
        //printf("!!");
        extern led_color_t g_color;
        g_color= (led_color_t){153, 0, 204};
    }
    else
    {
        extern led_color_t g_color;
        g_color= (led_color_t){15, 0, 20};
    }

    printf("%s\n", output_data);
        //printf("velocity_0: %d, velocity_1: %d, velocity_2: %d\r\n", velocity_0, velocity_1, velocity_2);
    vTaskDelay(pdMS_TO_TICKS(10));
}


