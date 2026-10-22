/**
 * @file    Key.c
 * @brief   4 键独立按键扫描 —— 短按/长按首次/持续长按
 *
 * Key_Scan() 需在 10ms 定时器中断中调用。低电平有效（上拉输入）。
 */

#include "ti_msp_dl_config.h"
#include "Key.h"

#define KEY1_PORT GPIO_Key_PIN_S1_PORT
#define KEY1_PIN  GPIO_Key_PIN_S1_PIN
#define KEY2_PORT GPIO_Key_PIN_S2_PORT
#define KEY2_PIN  GPIO_Key_PIN_S2_PIN
#define KEY3_PORT GPIO_Key_PIN_S3_PORT
#define KEY3_PIN  GPIO_Key_PIN_S3_PIN
#define KEY4_PORT GPIO_Key_PIN_S4_PORT
#define KEY4_PIN  GPIO_Key_PIN_S4_PIN

static uint16_t key_cnt[KEY_NUM] = {0};
static uint8_t key_flag[KEY_NUM] = {0};
static uint8_t key_hold[KEY_NUM] = {0};
static uint8_t key_long_flag[KEY_NUM] = {0};

static uint8_t ReadKey(uint8_t id)
{
    switch (id) {
        case KEY1:
            return (DL_GPIO_readPins(KEY1_PORT, KEY1_PIN) == 0U);
        case KEY2:
            return (DL_GPIO_readPins(KEY2_PORT, KEY2_PIN) == 0U);
        case KEY3:
            return (DL_GPIO_readPins(KEY3_PORT, KEY3_PIN) == 0U);
        case KEY4:
            return (DL_GPIO_readPins(KEY4_PORT, KEY4_PIN) == 0U);
        default:
            return 0U;
    }
}

void Key_Init(void)
{
    for (uint8_t i = 0; i < KEY_NUM; i++) {
        key_cnt[i] = 0U;
        key_flag[i] = 0U;
        key_hold[i] = 0U;
        key_long_flag[i] = 0U;
    }
}

void Key_Scan(void)
{
    for (uint8_t i = 0; i < KEY_NUM; i++) {
        if (ReadKey(i)) {
            if (key_cnt[i] < 0xFFFFU) {
                key_cnt[i]++;
            }

            /* 首次达长按阈值 */
            if (key_cnt[i] == LONG_PRESS_TIME) {
                key_hold[i] = 1U;
                key_long_flag[i] = 1U;
            }

            /* 首次之后每隔 LONG_PRESS_PERIOD 周期触发一次 */
            if ((key_cnt[i] > LONG_PRESS_TIME) &&
                (((key_cnt[i] - LONG_PRESS_TIME) % LONG_PRESS_PERIOD) == 0U)) {
                key_hold[i] = 1U;
            }
        } else {
            /* 短按: 至少 2 次确认且未达长按阈值 */
            if ((key_cnt[i] > 1U) && (key_cnt[i] < LONG_PRESS_TIME)) {
                key_flag[i] = 1U;
            }

            key_cnt[i] = 0U;
            key_hold[i] = 0U;
            key_long_flag[i] = 0U;
        }
    }
}

KeyPressState Key_Get(Key_ID_t key_id)
{
    if (key_id >= KEY_NUM) {
        return KEY_NONE;
    }

    if (key_flag[key_id] != 0U) {
        key_flag[key_id] = 0U;
        return KEY_SHORT_PRESS;
    }

    return KEY_NONE;
}

uint8_t Key_IsHold(Key_ID_t key_id)
{
    if (key_id >= KEY_NUM) {
        return 0U;
    }

    if (key_hold[key_id] != 0U) {
        key_hold[key_id] = 0U;
        return 1U;
    }

    return 0U;
}

uint8_t Key_IsLongPressed(Key_ID_t key_id)
{
    if (key_id >= KEY_NUM) {
        return 0U;
    }

    if (key_long_flag[key_id] != 0U) {
        key_long_flag[key_id] = 0U;
        return 1U;
    }

    return 0U;
}
