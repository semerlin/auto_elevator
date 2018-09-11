/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "stm32f10x_tim.h"
#include "stm32f10x_map.h"
#include "stm32f10x_cfg.h"
#include "assert.h"

/* flash register structure */
typedef struct
{
    volatile uint16_t CR1;
    uint16_t RESERVED1;
    volatile uint16_t CR2;
    uint16_t RESERVED2;
    volatile uint16_t SMCR;
    uint16_t RESERVED3;
    volatile uint16_t DIER;
    uint16_t RESERVED4;
    volatile uint16_t SR;
    uint16_t RESERVED5;
    volatile uint16_t EGR;
    uint16_t RESERVED6;
    volatile uint16_t CCMR1;
    uint16_t RESERVED7;
    volatile uint16_t CCMR2;
    uint16_t RESERVED8;
    volatile uint16_t CCER;
    uint16_t RESERVED9;
    volatile uint16_t CNT;
    uint16_t RESERVED10;
    volatile uint16_t PSC;
    uint16_t RESERVED11;
    volatile uint16_t ARR;
    uint16_t RESERVED12;
    uint32_t RESERVED13;
    volatile uint16_t CCR1;
    uint16_t RESERVED14;
    volatile uint16_t CCR2;
    uint16_t RESERVED15;
    volatile uint16_t CCR3;
    uint16_t RESERVED16;
    volatile uint16_t CCR4;
    uint16_t RESERVED17;
    uint32_t RESERVED18;
    volatile uint16_t DCR;
    uint16_t RESERVED19;
    volatile uint16_t DMAR;
    uint16_t RESERVED20;
} TIM_T;

#define CEN    (1 << 0)
#define DIR    (1 << 4)

/* GPIO group array */
static TIM_T *const TIMx[] = {(TIM_T *)TIM2_BASE,
                              (TIM_T *)TIM3_BASE,
                              (TIM_T *)TIM4_BASE,
                              (TIM_T *)TIM5_BASE
                             };

/**
 * @brief enable timer counter
 * @param group - timer group
 * @param flag - TRUE: enable FALSE: disable
 */
void TIM_Enable(TIM_Group group, bool flag)
{
    assert_param(group < TIM_Count);
    TIM_T *const tim = TIMx[group];

    if (flag)
    {
        tim->CR1 |= CEN;
    }
    else
    {
        tim->CR1 &= ~CEN;
    }
}

/**
 * @brief set count interval
 * @brief group - timer group
 * @brief interval - timer interval(1us)
 */
void TIM_SetCntInterval(TIM_Group group, uint32_t interval)
{
    assert_param(group < TIM_Count);
    TIM_T *const tim = TIMx[group];

    uint32_t pclk1 = RCC_GetPCLK1();
    uint16_t scale = (uint16_t)(interval * (pclk1 / 1000000));
    tim->PSC = scale - 1;
}

/**
 * @brief set timer auto reload value
 * @param group - timer group
 * @param reload - reload value
 */
void TIM_SetAutoReload(TIM_Group group, uint16_t reload)
{
    assert_param(group < TIM_Count);
    TIM_T *const tim = TIMx[group];

    tim->ARR = reload;
}

/**
 * @brief set timer count mode
 * @param group - timer group
 * @param mode - timer count mode
 */
void TIM_SetCountMode(TIM_Group group, uint8_t mode)
{
    assert_param(group < TIM_Count);
    assert_param(IS_TIM_COUNTMODE(mode));

    TIM_T *const tim = TIMx[group];
    tim->CR1 &= ~DIR;
    tim->CR1 |= mode;
}

/**
 * @brief enable or diable timer interrupt
 * @param group - timer group
 * @param itp - interrupt
 * @param flag - TRUE: enable, FALSE: disabel
 */
void TIM_IntEnable(TIM_Group group, uint16_t itp, bool flag)
{
    assert_param(group < TIM_Count);
    assert_param(IS_TIM_INT(itp));

    TIM_T *const tim = TIMx[group];
    if (flag)
    {
        tim->DIER |= itp;
    }
    else
    {
        tim->DIER &= ~itp;
    }
}

/**
 * @brief clear interrupt flag
 * @param group - timer group
 * @param itp - interrupt flag
 */
void TIM_ClearIntFlag(TIM_Group group, uint16_t itp)
{
    assert_param(group < TIM_Count);
    assert_param(IS_TIM_INT_FLAG(itp));

    TIM_T *const tim = TIMx[group];
    tim->SR &= ~itp;
}

/**
 * @brief get interrupt flag
 * @param group - timer group
 * @param itp - interrupt flag
 * @return interrupt flag set or not set
 */
bool TIM_IsInitSet(TIM_Group group, uint16_t itp)
{
    assert_param(group < TIM_Count);
    assert_param(IS_TIM_INT_FLAG(itp));

    TIM_T *const tim = TIMx[group];
    return (0 != (tim->SR & itp));
}

/**
 * @brief clear timer counter value
 * @param group - timer group
 */
void TIM_ClearCountValue(TIM_Group group)
{
    assert_param(group < TIM_Count);
    TIM_T *const  tim = TIMx[group];

    tim->CNT = 0;
}
