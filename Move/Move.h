/**
 * @file    Move.h
 * @brief   基本运动指令集接口
 */

#ifndef __MOVE_H
#define __MOVE_H

#include <stdint.h>

extern int16_t TargetspeedL;
extern int16_t TargetspeedR;

void Move_Init(void);
void Go_Ahead(void);
void Go_Back(void);
void Move_Stop(void);
void Turn_Left(void);
void Turn_Right(void);
void Turn_Back(void);

#endif /* __MOVE_H */
