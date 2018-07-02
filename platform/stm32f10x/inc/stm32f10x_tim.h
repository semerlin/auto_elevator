/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _STM32F10X_TIM_H_
  #define _STM32F10X_TIM_H_

#include "types.h"

typedef enum
{
    TIM2,
    TIM3,
    TIM4,
    TIM5,
    TIM_Count,
}TIM_Group;

#define TIM_COUNTMODE_UP     0
#define TIM_COUNTMODE_DOWN   (1 << 4)
#define IS_TIM_COUNTMODE(OCUNTMODE) ((OCUNTMODE == TIM_COUNTMODE_UP) || \
                                     (OCUNTMODE == TIM_COUNTMODE_DOWN))


#define TIM_INT_UPDATE         (1 << 0)
#define IS_TIM_INT(INT) ((INT == TIM_INT_UPDATE))

#define TIM_INT_FLAG_UPDATE     (1 << 0)
#define IS_TIM_INT_FLAG(FLAG) ((FLAG == TIM_INT_FLAG_UPDATE))

/* interface */
void TIM_Enable(TIM_Group group, bool flag);
void TIM_SetCntInterval(TIM_Group group, uint32_t interval);
void TIM_SetCountMode(TIM_Group group, uint8_t mode);
void TIM_IntEnable(TIM_Group group, uint16_t itp, bool flag);
void TIM_ClearIntFlag(TIM_Group group, uint16_t itp);
bool TIM_IsInitSet(TIM_Group group, uint16_t itp);
void TIM_ClearCountValue(TIM_Group group);
void TIM_SetAutoReload(TIM_Group group, uint16_t reload);

#endif /* _STM32F10x_TIM_H_ */