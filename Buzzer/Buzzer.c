/**
 * @file    Buzzer.c
 * @brief   蜂鸣器驱动 —— PB21 有源蜂鸣器，高电平响
 */

#include "ti_msp_dl_config.h"
#include "Buzzer.h"

void Buzzer_Start(void)
{
    DL_GPIO_setPins(GPIO_Buzzer_PORT, GPIO_Buzzer_PIN_bilibili_PIN);
}

void Buzzer_Stop(void)
{
    DL_GPIO_clearPins(GPIO_Buzzer_PORT, GPIO_Buzzer_PIN_bilibili_PIN);
}
