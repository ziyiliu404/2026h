/**
 * @file    Pid.c
 * @brief   位置式 PID 控制器（死区 + 积分抗饱和 + 输出限幅）
 *
 * Out = Kp*Error0 + Ki*ErrorInt + Kd*(Error0 - Error1)
 */

#include "Pid.h"

void PID_Clear(PID_t *p)
{
    p->Error0 = 0;
    p->Error1 = 0;
    p->ErrorInt = 0;
    p->Out = 0;
}

void PID_Update(PID_t *p)
{
    float integral_candidate;
    float output_candidate;

    p->Error1 = p->Error0;
    p->Error0 = p->Target - p->Actual;

    /* 死区 */
    if(p->Error0 < p->DeadBand &&
       p->Error0 > -p->DeadBand)
    {
        p->Error0 = 0;
    }

    /* 积分抗饱和: 输出已饱和且误差继续朝饱和方向时冻结积分 */
    if(p->Ki != 0)
    {
        integral_candidate = p->ErrorInt + p->Error0;

        if(integral_candidate > p->IntegralMax)
            integral_candidate = p->IntegralMax;

        if(integral_candidate < -p->IntegralMax)
            integral_candidate = -p->IntegralMax;

        output_candidate =
                p->Kp * p->Error0
              + p->Ki * integral_candidate
              + p->Kd * (p->Error0 - p->Error1);

        if (!((output_candidate > p->OutMax && p->Error0 > 0.0f) ||
              (output_candidate < p->OutMin && p->Error0 < 0.0f)))
        {
            p->ErrorInt = integral_candidate;
        }
    }
    else
    {
        p->ErrorInt = 0;
    }

    p->Out =
            p->Kp * p->Error0
          + p->Ki * p->ErrorInt
          + p->Kd * (p->Error0 - p->Error1);

    if(p->Out > p->OutMax)
        p->Out = p->OutMax;

    if(p->Out < p->OutMin)
        p->Out = p->OutMin;
}
