/**
 * @file    PWM_Car.h
 * @brief   PWM 模块对外接口
 */

#ifndef _PWM_CAR_H__
#define _PWM_CAR_H__

#include <stdint.h>

void PWM_Car_Init();
void Right_PWM(int16_t Compare);
void Left_PWM(int16_t Compare);
void Set_Freq(uint32_t freq);

#endif /* _PWM_CAR_H__ */
