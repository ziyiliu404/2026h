/**
 * @file    Key.h
 * @brief   4 键独立按键扫描接口
 *
 * 时间参数基于 10ms 扫描周期: LONG_PRESS_TIME=100(1s), LONG_PRESS_PERIOD=5(50ms)
 */

#ifndef __KEY_MSPM0_H
#define __KEY_MSPM0_H

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

#define KEY_NUM           4
#define LONG_PRESS_TIME   100
#define LONG_PRESS_PERIOD 5

typedef enum {
    KEY_NONE = 0,
    KEY_SHORT_PRESS,
} KeyPressState;

typedef enum {
    KEY1 = 0,
    KEY2,
    KEY3,
    KEY4
} Key_ID_t;

void Key_Init(void);
void Key_Scan(void);   /* 需在 10ms 定时器中断中调用 */
KeyPressState Key_Get(Key_ID_t key_id);
uint8_t Key_IsHold(Key_ID_t key_id);
uint8_t Key_IsLongPressed(Key_ID_t key_id);

#endif /* __KEY_MSPM0_H */
