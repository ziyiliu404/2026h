/**
 * @file    Pid.h
 * @brief   位置式 PID 控制器接口
 */

#ifndef PID_H
#define PID_H

typedef struct
{
    float Target;
    float Actual;
    float Error0;
    float Error1;
    float ErrorInt;
    float Kp;
    float Ki;
    float Kd;
    float Out;
    float OutMax;
    float OutMin;
    float IntegralMax;
    float DeadBand;
} PID_t;

void PID_Update(PID_t *p);
void PID_Clear(PID_t *p);

#endif /* PID_H */
