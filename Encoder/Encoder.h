/**
 * @file    Encoder.h
 * @brief   编码器 4x 正交解码接口
 *
 * MG513X: 13线 * 28减速比，4x 解码后每圈 1456 PPR，轮径 4.8cm
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#define WHEEL_DIAMETER_CM       4.8f
#define ENCODER_PPR             364
#define ENCODER_MULTIPLIER      4
#define ENCODER_PULSES_PER_REV  (ENCODER_PPR * ENCODER_MULTIPLIER)

typedef enum {
    FORWARD = 0,
    REVERSAL = 1
} ENCODER_DIR;

typedef struct {
    volatile int32_t count;
    volatile int32_t temp_count;   /* ISR 累加器, 10ms 快照清零 */
    ENCODER_DIR dir;
    int32_t speed;                 /* 脉冲/秒 */
    float distance_cm;
    float angle_deg;
} ENCODER_RES;

void encoder_init(void);
void encoder_update(void);

int32_t get_encoder_count_left(void);
int32_t get_encoder_count_right(void);
ENCODER_DIR get_encoder_dir_left(void);
ENCODER_DIR get_encoder_dir_right(void);
int32_t get_encoder_speed_left(void);
int32_t get_encoder_speed_right(void);

int16_t get_normalized_speed_left(void);
int16_t get_normalized_speed_right(void);

float get_distance_left_cm(void);
float get_distance_right_cm(void);
float get_speed_left_cmps(void);
float get_speed_right_cmps(void);

void reset_encoder_count(void);
void reset_encoder_distance(void);

void GROUP1_IRQHandler(void);

#endif /* ENCODER_H */
