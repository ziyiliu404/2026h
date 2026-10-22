#ifndef __SERVO_H__
#define __SERVO_H__

#include <stdint.h>

#define SERVO1_MAX_ANGLE 180U
#define SERVO2_MAX_ANGLE 270U

typedef enum
{
    SERVO_1 = 0,
    SERVO_2
} Servo_ID_t;

void Servo_Init(void);
void Servo_SetAngle(Servo_ID_t servo, uint16_t angle);
void Servo_SetServo1Angle(uint16_t angle);
void Servo_SetServo2Angle(uint16_t angle);
uint16_t Servo_GetAngle(Servo_ID_t servo);

#endif
