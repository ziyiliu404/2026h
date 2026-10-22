/**
 * @file    Move.c
 * @brief   基本运动指令集，修改 TargetspeedL/R，由 Motor_Set 输出
 */

#include "ti_msp_dl_config.h"
#include "Motor.h"

#define speed 35

int16_t TargetspeedL = speed;
int16_t TargetspeedR = speed;

void Go_Ahead(void)
{
    TargetspeedL = speed;
    TargetspeedR = speed;
}

void Go_Back(void)
{
    TargetspeedL = -speed;
    TargetspeedR = -speed;
}

void Move_Stop(void)
{
    TargetspeedL = 0;
    TargetspeedR = 0;
}

void Turn_Left(void)
{
    TargetspeedL = 0;
    TargetspeedR = speed;
}

void Turn_Right(void)
{
    TargetspeedL = speed;
    TargetspeedR = 0;
}

void Turn_Back(void)
{
    TargetspeedL = -speed;
    TargetspeedR = speed;
}
