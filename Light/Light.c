/**
 * @file    Light.c
 * @brief   CD4051 8路灰度传感器驱动程序 + 循迹查表
 *
 * 通过 CD4051 8选1模拟开关轮询读取 8 路灰度传感器，打包为 8-bit 位图
 * sensor_raw（bit7=x1 最左，bit0=x8 最右），再经查表映射为循迹误差 err。
 * 负数 err = 线偏左需右转，正数 = 线偏右需左转。
 */

#include "ti_msp_dl_config.h"
#include "Light.h"

uint8_t line_detected_flag = 0U;   /* 1=检测到黑线, 0=丢线 */
uint8_t mark_flag = 0U;            /* 1=检测到路口标记 */
uint8_t sensor_raw = 0U;           /* 8 路传感器位图 (bit7=最左) */
float err = 0.0f;

/* 8 路灰度传感器读值，物理顺序 x1(最左) ~ x8(最右) */
uint8_t x1;
uint8_t x2;
uint8_t x3;
uint8_t x4;
uint8_t x5;
uint8_t x6;
uint8_t x7;
uint8_t x8;

/* 路口标记 = 横向黑线（中间连续 4~6 路见黑） */
static uint8_t is_mark_pattern(uint8_t sensor)
{
    switch (sensor)
    {
        case 0b00111100:
        case 0b01111110:
        case 0b01111100:
        case 0b00111110:
            return 1U;

        default:
            return 0U;
    }
}

/* 轮询 CD4051 8 个通道，AD0/AD1/AD2 选通，读输出脚存入 x1~x8 */
void Light_Get(void)
{
    uint8_t *const sensors[8] = {&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8};

    const uint32_t address_mask =
        GPIO_Follow_AD0_PIN | GPIO_Follow_AD1_PIN | GPIO_Follow_AD2_PIN;

    for (uint8_t channel = 0U; channel < 8U; channel++)
    {
        uint32_t set_mask = 0U;

        if ((channel & 0x01U) != 0U)
        {
            set_mask |= GPIO_Follow_AD0_PIN;
        }
        if ((channel & 0x02U) != 0U)
        {
            set_mask |= GPIO_Follow_AD1_PIN;
        }
        if ((channel & 0x04U) != 0U)
        {
            set_mask |= GPIO_Follow_AD2_PIN;
        }

        /* 先清后设，避免地址线切换毛刺 */
        DL_GPIO_clearPins(GPIO_Follow_PORT, address_mask);
        DL_GPIO_setPins(GPIO_Follow_PORT, set_mask);

        delay_cycles(CPUCLK_FREQ / 10000U);   /* ~100us 建立时间 */

        *sensors[channel] =
            DL_GPIO_readPins(GPIO_Follow_PORT, GPIO_Follow_OUT_PIN) ? 1U : 0U;
    }
}

/* 打包位图 -> 标记识别 -> 查表计算误差 */
void Track_Process(void)
{
    Light_Get();

    sensor_raw = (uint8_t) ((x1 << 7) | (x2 << 6) | (x3 << 5) | (x4 << 4) |
                            (x5 << 3) | (x6 << 2) | (x7 << 1) | x8);

    line_detected_flag = 0U;
    mark_flag = is_mark_pattern(sensor_raw);

    /* 查表: 位图 -> err。负=偏左需右转, 正=偏右需左转 */
    switch (sensor_raw)
    {
        case 0b00011000:
            err = 0.0f;
            line_detected_flag = 1U;
            break;

        case 0b00010000:
            err = 10.0f;
            line_detected_flag = 1U;
            break;

        case 0b00100000:
        case 0b00110000:
        case 0b00101000:
        case 0b00111000:
            err = 10.0f;
            line_detected_flag = 1U;
            break;

        case 0b01000000:
        case 0b01100000:
        case 0b01110000:
        case 0b01111000:
        case 0b01011000:
        case 0b01101000:
            err = 10.0f;
            line_detected_flag = 1U;
            break;

        case 0b10000000:
        case 0b11000000:
        case 0b11100000:
        case 0b11110000:
        case 0b11111000:
        case 0b10100000:
            err = 10.0f;
            line_detected_flag = 1U;
            break;

        case 0b00001000:
            err = 0.0f;
            line_detected_flag = 1U;
            break;

        case 0b00000100:
        case 0b00001100:
        case 0b00010100:
        case 0b00011100:
            err = 0.0f;
            line_detected_flag = 1U;
            break;

        case 0b00000010:
        case 0b00000110:
        case 0b00001110:
        case 0b00011110:
        case 0b00001010:
        case 0b00010110:
            err = 0.0f;
            line_detected_flag = 1U;
            break;

        case 0b00000001:
        case 0b00000011:
        case 0b00000111:
        case 0b00001111:
        case 0b00011111:
        case 0b00000101:
            err = 0.0f;
            line_detected_flag = 1U;
            break;

        case 0b00000000:            /* 全部白: 丢线 */
            err = 0.0f;
            line_detected_flag = 0U;
            break;

        case 0b11111111:            /* 全部黑: 十字/路口 */
            err = 0.0f;
            line_detected_flag = 1U;
            break;

        default:
            err = 0.0f;
            break;
    }

    /* 兜底: 任意传感器见黑即认为有线 */
    if (sensor_raw != 0U)
    {
        line_detected_flag = 1U;
    }
}
