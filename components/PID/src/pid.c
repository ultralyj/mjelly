#include <stdio.h>
#include <stdlib.h>
#include "motor.h"
#include "bdc_motor.h"

#define Kp 2.909914  // 增大比例增益
#define Ki 0.13   // 积分增益
#define Kd 0.24   // 微分增益


// 目标速度（单位：转/分钟或其他单位）
// 速度值（脉冲数差）
extern volatile int velocity_0;
extern volatile int velocity_1;
extern volatile int velocity_2;
//int target_velocity = 2700; // 目标速度，可以通过设置改变
extern motor_control_context_t motor_ctrl_ctx[3];

// 当前速度（需要通过编码器计算得到）
//volatile int current_velocity = 0;

// PID控制器变量
int previous_error = 0;    // 上次误差
int integral = 0;          // 积分项
int derivative = 0;        // 微分项
int pid_output = 0;        // PID输出


// PID控制算法
void pid_control(void) {
    // 获取当前速度
    int target_velocity = -1900; // 目标速度，可以通过设置改变

    int current_velocity = velocity_2;

    // 计算误差
    int error = target_velocity - current_velocity;

    // 计算积分项，累积误差（避免长期误差）
    integral += error;
    if (integral > 1000) integral = 1000;   // 限制最大积分项
    if (integral < -1000) integral = -1000; // 限制最小积分项

    // 计算微分项，得到误差变化率
    derivative = error - previous_error;

    // PID控制输出
    pid_output = ((Kp * error) + (Ki * integral) + (Kd * derivative));

    // 控制输出范围限制（避免超出电机驱动器的输入范围）
    //set_motor_speed(pid_output);

    if (pid_output > 400) {
        pid_output = 400;  // 最大速度限制
    } else if (pid_output < -400) {
        pid_output = -400; // 最小速度限制
    } 

    // 输出控制信号给电机
    bdc_motor_set_speed(motor_ctrl_ctx[2].motor, pid_output);  // 假设这是停止电机的函数
    //printf("Error: %d, PID Output: %d\n", error, pid_output);
    // 更新上一时刻的误差
    previous_error = error;

    printf("PID Output: %d, Error: %d, Current Velocity: %d\n", pid_output, error, current_velocity);
}




