/**
 * @file    Light.h
 * @brief   CD4051 8路灰度传感器模块接口
 */

#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

/* 8 路传感器读值，x1 最左 ~ x8 最右 */
extern uint8_t x1;
extern uint8_t x2;
extern uint8_t x3;
extern uint8_t x4;
extern uint8_t x5;
extern uint8_t x6;
extern uint8_t x7;
extern uint8_t x8;

extern float err;                  /* 循迹误差，负=偏左需右转 */
extern uint8_t line_detected_flag; /* 1=在线, 0=丢线 */
extern uint8_t mark_flag;          /* 1=检测到路口 */
extern uint8_t sensor_raw;         /* 8 路位图 (bit7=x1) */

void Light_Get(void);              /* 轮询读取 CD4051 8 通道 */
void Track_Process(void);          /* 打包位图 -> 标记识别 -> 查表计算误差 */

#endif /* LIGHT_H */
