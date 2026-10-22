/**
 * @file    PWM_Car.c
 * @brief   PWM 定时器初始化与底层 CCR 控制
 *
 * 右轮: TIMA0 (PWM_Car_INST) CCP0; 左轮: TIMG7 (PWM_Car_Left_INST) CCP1
 */

#include "ti_msp_dl_config.h"
#include "PWM_Car.h"

void PWM_Car_Init()
{
    DL_Timer_startCounter(PWM_Car_INST);
    DL_Timer_startCounter(PWM_Car_Left_INST);
}

void Right_PWM(int16_t compare)
{
    DL_TimerA_setCaptureCompareValue(
        PWM_Car_INST, compare, GPIO_PWM_Car_C0_IDX);
}

void Left_PWM(int16_t compare)
{
    DL_TimerG_setCaptureCompareValue(
        PWM_Car_Left_INST, compare, GPIO_PWM_Car_Left_C1_IDX);
}

/* period = CLK_FREQ / freq，同步写入两个定时器 */
void Set_Freq(uint32_t freq)
{
    uint32_t period = PWM_Car_INST_CLK_FREQ / freq;

    DL_Timer_setLoadValue(PWM_Car_INST, period);
    DL_Timer_setLoadValue(PWM_Car_Left_INST, period);
}
