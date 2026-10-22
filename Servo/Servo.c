#include "Servo.h"

#include "ti_msp_dl_config.h"

#define SERVO_PWM_PERIOD_TICKS 1000U
#define SERVO_MIN_PULSE_TICKS   25U
#define SERVO_MAX_PULSE_TICKS  125U

static uint16_t g_servo_angle[2] = {0U, 0U};

static uint16_t Servo_GetMaxAngle(Servo_ID_t servo)
{
    return (servo == SERVO_2) ? SERVO2_MAX_ANGLE : SERVO1_MAX_ANGLE;
}

static uint16_t Servo_ClampAngle(Servo_ID_t servo, uint16_t angle)
{
    uint16_t max_angle = Servo_GetMaxAngle(servo);

    if (angle > max_angle) {
        angle = max_angle;
    }

    return angle;
}

static uint32_t Servo_AngleToCompare(Servo_ID_t servo, uint16_t angle)
{
    uint16_t max_angle = Servo_GetMaxAngle(servo);
    uint32_t pulse_ticks;

    angle = Servo_ClampAngle(servo, angle);

    pulse_ticks = SERVO_MIN_PULSE_TICKS +
                  (((uint32_t) angle * (SERVO_MAX_PULSE_TICKS - SERVO_MIN_PULSE_TICKS)) +
                   (max_angle / 2U)) / max_angle;

    return SERVO_PWM_PERIOD_TICKS - pulse_ticks;
}

void Servo_Init(void)
{
    g_servo_angle[0] = 0U;
    g_servo_angle[1] = 0U;

    Servo_SetServo1Angle(0U);
    Servo_SetServo2Angle(0U);

    DL_TimerG_startCounter(PWM_Servo_INST);
}

void Servo_SetAngle(Servo_ID_t servo, uint16_t angle)
{
    uint32_t compare;

    if (servo > SERVO_2) {
        return;
    }

    angle = Servo_ClampAngle(servo, angle);
    compare = Servo_AngleToCompare(servo, angle);
    g_servo_angle[servo] = angle;

    if (servo == SERVO_1) {
        DL_TimerG_setCaptureCompareValue(PWM_Servo_INST, compare, GPIO_PWM_Servo_C0_IDX);
    } else {
        DL_TimerG_setCaptureCompareValue(PWM_Servo_INST, compare, GPIO_PWM_Servo_C1_IDX);
    }
}

void Servo_SetServo1Angle(uint16_t angle)
{
    Servo_SetAngle(SERVO_1, angle);
}

void Servo_SetServo2Angle(uint16_t angle)
{
    Servo_SetAngle(SERVO_2, angle);
}

uint16_t Servo_GetAngle(Servo_ID_t servo)
{
    if (servo > SERVO_2) {
        return 0U;
    }

    return g_servo_angle[servo];
}
