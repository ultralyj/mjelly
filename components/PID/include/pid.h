#ifndef PID_H
#define PID_H

// PID控制器参数定义
// #define Kp 2.0
// #define Ki 0.5
// #define Kd 1.0

// 目标速度（单位：转/分钟或其他单位）
extern int target_velocity; // 目标速度

// 速度值（脉冲数差）
extern volatile int velocity_0;
extern volatile int velocity_1;
extern volatile int velocity_2;

// PID控制函数
void pid_control(void);

#endif
