/**
 * @file    Encoder.c
 * @brief   MG513X 编码器 4x 正交解码
 *
 * 左轮 A1=PB10/B1=PB11, 右轮 A2=PB12/B2=PB19，在 GPIOB 中断 (GROUP1_IRQHandler)
 * 中累加 temp_count，每 10ms 由 encoder_update() 快照并计算速度/里程。
 * 右轮机械镜像安装，delta 取反使左右轮"正=前进"语义一致。
 */

#include "Encoder.h"
#include "ti_msp_dl_config.h"

#define SPEED_SCALE_LEFT          (1800.0f / 30.0f)
#define SPEED_SCALE_RIGHT         (1800.0f / 30.0f)

#define ENCODER_INTERVAL_MS       10
#define WHEEL_CIRCUMFERENCE_CM    (3.1415926f * WHEEL_DIAMETER_CM)
#define PULSE_TO_CM               (WHEEL_CIRCUMFERENCE_CM / ENCODER_PULSES_PER_REV)

static ENCODER_RES motor_encoder_left = {0};
static ENCODER_RES motor_encoder_right = {0};

void encoder_init(void)
{
    NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
}

void encoder_update(void)
{
    int32_t left_delta;
    int32_t right_delta;

    uint32_t interrupt_state = __get_PRIMASK();

    /* 临界区: 原子读取并清零 temp_count */
    __disable_irq();
    left_delta = motor_encoder_left.temp_count;
    right_delta = motor_encoder_right.temp_count;
    motor_encoder_left.temp_count = 0;
    motor_encoder_right.temp_count = 0;
    if (interrupt_state == 0U)
    {
        __enable_irq();
    }

    motor_encoder_left.count += left_delta;
    motor_encoder_left.distance_cm += left_delta * PULSE_TO_CM;
    motor_encoder_left.speed = (left_delta * 1000) / ENCODER_INTERVAL_MS;
    motor_encoder_left.dir = (left_delta >= 0) ? FORWARD : REVERSAL;

    /* 右轮镜像安装，delta 取反统一"正=前进" */
    motor_encoder_right.count -= right_delta;
    motor_encoder_right.distance_cm -= right_delta * PULSE_TO_CM;
    motor_encoder_right.speed = (-right_delta * 1000) / ENCODER_INTERVAL_MS;
    motor_encoder_right.dir = (right_delta <= 0) ? FORWARD : REVERSAL;
}

int32_t get_encoder_count_left(void) { return motor_encoder_left.count; }
int32_t get_encoder_count_right(void) { return motor_encoder_right.count; }
ENCODER_DIR get_encoder_dir_left(void) { return motor_encoder_left.dir; }
ENCODER_DIR get_encoder_dir_right(void) { return motor_encoder_right.dir; }
int32_t get_encoder_speed_left(void) { return motor_encoder_left.speed; }
int32_t get_encoder_speed_right(void) { return motor_encoder_right.speed; }

float get_distance_left_cm(void) { return motor_encoder_left.distance_cm; }
float get_distance_right_cm(void) { return motor_encoder_right.distance_cm; }
float get_speed_left_cmps(void) { return motor_encoder_left.speed * PULSE_TO_CM; }
float get_speed_right_cmps(void) { return motor_encoder_right.speed * PULSE_TO_CM; }

int16_t get_normalized_speed_left(void)
{
    float speed = (float) motor_encoder_left.speed / SPEED_SCALE_LEFT;
    if (speed > 100.0f) speed = 100.0f;
    if (speed < -100.0f) speed = -100.0f;
    return (int16_t) speed;
}

int16_t get_normalized_speed_right(void)
{
    float speed = (float) motor_encoder_right.speed / SPEED_SCALE_RIGHT;
    if (speed > 100.0f) speed = 100.0f;
    if (speed < -100.0f) speed = -100.0f;
    return (int16_t) speed;
}

/* GPIOB 中断: 4x 正交解码。A 相沿读 B 相电平判方向，反之亦然 */
void GROUP1_IRQHandler(void)
{
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(
        GPIOB,
        DL_GPIO_PIN_10 | DL_GPIO_PIN_11 | DL_GPIO_PIN_12 | DL_GPIO_PIN_19
    );

    if (status & DL_GPIO_PIN_10)
    {
        motor_encoder_left.temp_count +=
            DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_11) ? 1 : -1;
        DL_GPIO_clearInterruptStatus(GPIOB, DL_GPIO_PIN_10);
    }

    if (status & DL_GPIO_PIN_11)
    {
        motor_encoder_left.temp_count +=
            DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_10) ? -1 : 1;
        DL_GPIO_clearInterruptStatus(GPIOB, DL_GPIO_PIN_11);
    }

    if (status & DL_GPIO_PIN_12)
    {
        motor_encoder_right.temp_count +=
            DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_19) ? 1 : -1;
        DL_GPIO_clearInterruptStatus(GPIOB, DL_GPIO_PIN_12);
    }

    if (status & DL_GPIO_PIN_19)
    {
        motor_encoder_right.temp_count +=
            DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_12) ? -1 : 1;
        DL_GPIO_clearInterruptStatus(GPIOB, DL_GPIO_PIN_19);
    }
}

void reset_encoder_count(void)
{
    uint32_t interrupt_state = __get_PRIMASK();

    __disable_irq();
    motor_encoder_left.count = 0;
    motor_encoder_right.count = 0;
    motor_encoder_left.temp_count = 0;
    motor_encoder_right.temp_count = 0;
    if (interrupt_state == 0U)
    {
        __enable_irq();
    }
}

void reset_encoder_distance(void)
{
    motor_encoder_left.distance_cm = 0.0f;
    motor_encoder_right.distance_cm = 0.0f;
}
