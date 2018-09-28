/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _STM32F10x_CFG_H
#define _STM32F10x_CFG_H

#include "assert.h"
#include "trace.h"

/**********************************************************
* library module inclue configure
***********************************************************/
/*********************************************************/
#define _MODULE_RCC
#define _MODULE_FLASH
#define _MODULE_GPIO
#define _MODULE_USART
#define _MODULE_SCB
#define _MODULE_NVIC
#define _MODULE_SYSTICK
#define _MODULE_TIM
#define _MODULE_CAN
#define _MODULE_SIG

/**********************************************************/
#ifdef _MODULE_FLASH
#include "stm32f10x_flash.h"
#endif

#ifdef _MODULE_RCC
#include "stm32f10x_rcc.h"
#endif

#ifdef _MODULE_GPIO
#include "stm32f10x_gpio.h"
#endif

#ifdef _MODULE_USART
#include "stm32f10x_usart.h"
#endif

#ifdef _MODULE_SCB
#include "stm32f10x_scb.h"
#endif

#ifdef _MODULE_NVIC
#include "stm32f10x_nvic.h"
#endif

#ifdef _MODULE_SYSTICK
#include "stm32f10x_systick.h"
#endif

#ifdef _MODULE_TIM
#include "stm32f10x_tim.h"
#endif

#ifdef _MODULE_CAN
#include "stm32f10x_can.h"
#endif

#ifdef _MODULE_SIG
#include "stm32f10x_sig.h"
#endif

#endif /* _STM32F10x_CFG_H_ */
