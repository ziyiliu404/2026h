/**
 * @file    Motor.c
 * @brief   TB6612 双路直流电机驱动
 *
 * 左轮: PWM=TIMG7 CCP1, 方向 GPIO_Left1/Left2（接线交叉，方向逻辑与右轮相反）
 * 右轮: PWM=TIMA0 CCP0, 方向 GPIO_Right1/Right2
 * speed>0 前进, <0 后退, ==0 短接制动, 范围 [-100,100]
 */

#include "ti_msp_dl_config.h"
#include "Motor.h"
#include <stdlib.h>

void PWM_SetMotorA(int8_t speed)
{
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    uint32_t period = DL_TimerG_getLoadValue(PWM_Car_Left_INST);
    uint32_t ccr = (abs(speed) * period) / 100;

    DL_TimerG_setCaptureCompareValue(
        PWM_Car_Left_INST, ccr, GPIO_PWM_Car_Left_C1_IDX);
}

void PWM_SetMotorB(int8_t speed)
{
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    uint32_t period = DL_TimerA_getLoadValue(PWM_Car_INST);
    uint32_t ccr = (abs(speed) * period) / 100;

    DL_TimerA_setCaptureCompareValue(
        PWM_Car_INST, ccr, GPIO_PWM_Car_C0_IDX);
}

/* 左电机接线交叉，GPIO IN1/IN2 逻辑与右电机镜像 */
static void Car_SetLDirection(int8_t speed)
{
    if (speed > 0)
    {
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Left1_PIN);
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Left2_PIN);
    }
    else if (speed < 0)
    {
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Left1_PIN);
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Left2_PIN);
    }
    else
    {
        /* 短接制动 */
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Left1_PIN);
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Left2_PIN);
    }
}

static void Car_SetRDirection(int8_t speed)
{
    if (speed > 0)
    {
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Right1_PIN);
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Right2_PIN);
    }
    else if (speed < 0)
    {
        DL_GPIO_clearPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Right1_PIN);
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Right2_PIN);
    }
    else
    {
        /* 短接制动 */
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Right1_PIN);
        DL_GPIO_setPins(GPIO_Motor_PORT, GPIO_Motor_PIN_Right2_PIN);
    }
}

void Motor_Set(int8_t left, int8_t right)
{
    if (left > 100)  left = 100;
    if (left < -100) left = -100;
    if (right > 100) right = 100;
    if (right < -100) right = -100;

    Car_SetLDirection(left);
    Car_SetRDirection(right);

    PWM_SetMotorA(left);
    PWM_SetMotorB(right);
}
