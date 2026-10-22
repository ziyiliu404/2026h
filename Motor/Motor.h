/**
 * @file    Motor.h
 * @brief   电机驱动对外接口
 */

#ifndef _MOTOR_H__
#define _MOTOR_H__

#include <stdint.h>

void PWM_SetMotorA(int8_t speed);
void PWM_SetMotorB(int8_t speed);
void Motor_Set(int8_t left, int8_t right);

#endif /* _MOTOR_H__ */
